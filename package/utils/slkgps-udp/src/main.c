#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <termios.h>
#include <signal.h>
#include <errno.h>
#include <pthread.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <time.h>
#include <math.h>
#include <sys/time.h>
#include <openssl/evp.h>
#include <openssl/rand.h>
#include <openssl/err.h>

// 配置参数 - 串口
//#define SERIAL_PORT "/dev/ttyMSM1"
char *SERIAL_PORT = "/dev/ttyS2";
#define BAUDRATE B115200
#define SERIAL_BUFFER_SIZE 1024
#define MAX_ERROR_COUNT 5
#define READ_TIMEOUT_MS 100

// 配置参数 - UDP
#define UDP_BUFFER_SIZE 4096
#define RECONNECT_DELAY 5
#define HEARTBEAT_INTERVAL 30
#define ID_SEPARATOR "|"

// 关键宏定义：数据包起始标志（支持GGA和RMC）
#define NMEA_GGA_FLAG "$GNGGA"
#define NMEA_RMC_FLAG "$GNRMC"
#define GGA_FLAG_LEN strlen(NMEA_GGA_FLAG)
#define RMC_FLAG_LEN strlen(NMEA_RMC_FLAG)

// 数据包优先级定义
#define PRIORITY_RMC 2    // RMC优先级最高
#define PRIORITY_GGA 1    // GGA优先级次之
#define PRIORITY_NONE 0   // 无优先级

// 全局变量
int UPLOAD_INTERVAL = 5;
int SEND_INTERVAL = 1;
int serial_fd = -1;
int sockfd = -1;
int running = 1;
pthread_mutex_t data_mutex = PTHREAD_MUTEX_INITIALIZER;
char packet_buffer[UDP_BUFFER_SIZE] = {0};
int packet_in_progress = 0;
int data_ready = 0;
char device_id[64] = {0};

// UDP相关变量
struct sockaddr_in server_addr;
char server_ip[64] = {0};
int server_port = 0;

// 时间间隔控制变量
time_t last_send_time = 0;

// 新增：当前数据包类型和优先级
char current_packet_type[16] = {0};
int current_packet_priority = PRIORITY_NONE;

// 函数声明
char *order(char* order);
int function(char *s);
void handle_signal(int sig);
int init_serial(const char *port);
int reconnect_serial();
int init_udp_socket();
int reconnect_server();
void load_device_id();
void load_server_info();
void load_send_interval();
void add_to_packet(const char *line);
void send_packet();
int can_send_now();
int is_nmea_start(const char *line);
int get_nmea_priority(const char *line);  // 新增：获取NMEA数据优先级
const char* get_nmea_type(const char *line);
void *serial_read_thread(void *arg);
void *upload_thread(void *arg);
size_t count_nmea_lines(const char *data);

// 执行Linux命令并返回结果字符串
char *order(char* order) {
    FILE *fp;
    char *s = (char *)malloc(1024);
    char *buffer = (char *)malloc(1024);
    memset(buffer, 0, 1024);
    fp = popen(order, "r");
    while (fgets(s, sizeof(s), fp) != NULL)
        strcat(buffer, s);
    pclose(fp);
    if (strlen(buffer) > 0)
        buffer[strlen(buffer) - 1] = '\0';
    free(s);
    return buffer;
}

// 执行Linux命令并返回数字结果
int function(char *s) {
    int value;
    char *buffer = order(s);
    value = atoi(buffer);
    free(buffer);
    return value;
}

// 从uci加载设备ID
void load_device_id() {
    char *id = order("uci -q get nport.@ttyS2[0].DeviceID");
    if (id != NULL && strlen(id) > 0) {
        strncpy(device_id, id, sizeof(device_id) - 1);
        printf("成功加载设备ID: %s\n", device_id);
    } else {
        strcpy(device_id, "DEFAULT_DEVICE_ID");
        printf("uci未配置DeviceID，使用默认ID: %s\n", device_id);
    }
    free(id);
}

// 加载服务器信息
void load_server_info() {
    char *ip = order("uci -q get nport.@ttyS2[0].serverip");
    int port = function("uci -q get nport.@ttyS2[0].serverport");
    
    if (ip != NULL && strlen(ip) > 0) {
        strncpy(server_ip, ip, sizeof(server_ip) - 1);
        server_port = port;
        printf("成功加载服务器信息: %s:%d\n", server_ip, server_port);
    } else {
        strcpy(server_ip, "127.0.0.1");
        server_port = 8080;
        printf("uci未配置服务器信息，使用默认: %s:%d\n", server_ip, server_port);
    }
    free(ip);
    
    // 初始化服务器地址结构
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(server_port);
    if (inet_pton(AF_INET, server_ip, &server_addr.sin_addr) <= 0) {
        perror("无效的服务器IP地址");
    }
}

// 加载发送间隔配置
void load_send_interval() {
    char *interval_str = order("uci -q get nport.@ttyS2[0].send_interval");
    if (interval_str != NULL && strlen(interval_str) > 0) {
        SEND_INTERVAL = atoi(interval_str);
        if (SEND_INTERVAL <= 0) {
            SEND_INTERVAL = 1;
            printf("发送间隔配置无效，使用默认值: %d秒\n", SEND_INTERVAL);
        } else {
            printf("成功加载发送间隔: %d秒\n", SEND_INTERVAL);
        }
    } else {
        SEND_INTERVAL = 1;
        printf("未配置发送间隔，使用默认值: %d秒\n", SEND_INTERVAL);
    }
    free(interval_str);
}

// 检查是否达到发送时间间隔
int can_send_now() {
    time_t current_time = time(NULL);
    
    if (last_send_time == 0) {
        last_send_time = current_time;
        return 1;
    }
    
    if (current_time - last_send_time >= SEND_INTERVAL) {
        last_send_time = current_time;
        return 1;
    }
    
    return 0;
}

// 检查是否是NMEA数据包开头（支持GGA和RMC）
int is_nmea_start(const char *line) {
    if (line == NULL || strlen(line) < 6) return 0;
 

    if (strncmp(line, NMEA_RMC_FLAG, RMC_FLAG_LEN) == 0) {
        return 1;
    } 
    if (strncmp(line, NMEA_GGA_FLAG, GGA_FLAG_LEN) == 0) {
        return 1;
    }
    
    return 0;
}

// 新增：获取NMEA数据优先级（RMC优先级高于GGA）
int get_nmea_priority(const char *line) {
    if (line == NULL || strlen(line) < 6) return PRIORITY_NONE;
    
    if (strncmp(line, NMEA_RMC_FLAG, RMC_FLAG_LEN) == 0) {
        return PRIORITY_RMC;  // RMC优先级最高
    }
    
    if (strncmp(line, NMEA_GGA_FLAG, GGA_FLAG_LEN) == 0) {
        return PRIORITY_GGA;  // GGA优先级次之
    }
    
    return PRIORITY_NONE;
}

// 获取NMEA数据类型
const char* get_nmea_type(const char *line) {
    if (line == NULL || strlen(line) < 6) return "未知";
    
    if (strncmp(line, NMEA_GGA_FLAG, GGA_FLAG_LEN) == 0) {
        return "GGA";
    }
    
    if (strncmp(line, NMEA_RMC_FLAG, RMC_FLAG_LEN) == 0) {
        return "RMC";
    }
    
    return "其他";
}

// 信号处理函数
void handle_signal(int sig) {
    running = 0;
    printf("\n收到退出信号，正在关闭程序...\n");
    pthread_mutex_lock(&data_mutex);
    if (packet_in_progress && strlen(packet_buffer) > 0) {
        send_packet();
    }
    pthread_mutex_unlock(&data_mutex);
}

// 串口初始化函数
int init_serial(const char *port) {
    int fd;
    struct termios tty;
    memset(&tty, 0, sizeof(tty));

    fd = open(port, O_RDWR | O_NOCTTY);
    if (fd == -1) {
        perror("无法打开串口设备");
        return -1;
    }

    if (tcgetattr(fd, &tty) != 0) {
        perror("获取串口属性失败");
        close(fd);
        return -1;
    }

    cfsetospeed(&tty, BAUDRATE);
    cfsetispeed(&tty, BAUDRATE);

    tty.c_cflag = (tty.c_cflag & ~CSIZE) | CS8;
    tty.c_iflag &= ~IGNBRK;
    tty.c_lflag = 0;
    tty.c_oflag = 0;
    tty.c_cc[VMIN] = 0;
    tty.c_cc[VTIME] = READ_TIMEOUT_MS / 100;

    tty.c_iflag &= ~(IXON | IXOFF | IXANY);
    tty.c_cflag |= (CLOCAL | CREAD);
    tty.c_cflag &= ~(PARENB | PARODD);
    tty.c_cflag &= ~CSTOPB;
    tty.c_cflag &= ~CRTSCTS;

    if (tcsetattr(fd, TCSANOW, &tty) != 0) {
        perror("应用串口配置失败");
        close(fd);
        return -1;
    }

    tcflush(fd, TCIFLUSH);
    printf("串口 %s 初始化成功，波特率: %d\n", port, BAUDRATE);
    return fd;
}

// 串口重连函数
int reconnect_serial() {
    printf("尝试重新连接串口...\n");
    
    if (serial_fd != -1) {
        close(serial_fd);
        serial_fd = -1;
    }
    
    for (int i = 0; i < 3; i++) {
        serial_fd = init_serial(SERIAL_PORT);
        if (serial_fd != -1) {
            return 1;
        }
        printf("串口重连失败，%d秒后重试...\n", i + 1);
        sleep(i + 1);
    }
    
    return 0;
}

// UDP socket初始化函数
int init_udp_socket() {
    int fd;
    
    if ((fd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
        perror("创建UDP套接字失败");
        return -1;
    }
    
    // 设置socket选项：允许广播
    int broadcast = 1;
    if (setsockopt(fd, SOL_SOCKET, SO_BROADCAST, &broadcast, sizeof(broadcast)) < 0) {
        perror("设置广播选项失败");
        close(fd);
        return -1;
    }
    
    // 设置发送超时
    struct timeval timeout;
    timeout.tv_sec = 5;
    timeout.tv_usec = 0;
    if (setsockopt(fd, SOL_SOCKET, SO_SNDTIMEO, &timeout, sizeof(timeout)) < 0) {
        perror("设置发送超时失败");
    }
    
    printf("UDP socket创建成功\n");
    return fd;
}

// UDP重连函数
int reconnect_server() {
    printf("尝试重新创建UDP连接...\n");
    
    if (sockfd != -1) {
        close(sockfd);
        sockfd = -1;
    }
    
    for (int i = 0; i < 3; i++) {
        sockfd = init_udp_socket();
        if (sockfd != -1) {
            return 1;
        }
        printf("UDP socket创建失败，%d秒后重试...\n", i + 1);
        sleep(i + 1);
    }
    
    return 0;
}

// 统计NMEA语句条数
size_t count_nmea_lines(const char *data) {
    if (data == NULL || strlen(data) == 0) return 0;
    size_t count = 0;
    const char *ptr = data;
    
    // 跳过ID部分
    ptr = strstr(ptr, ID_SEPARATOR);
    if (ptr == NULL) return 0;
    ptr += strlen(ID_SEPARATOR);
    
    // 统计换行符数量
    while ((ptr = strchr(ptr, '\n')) != NULL) {
        count++;
        ptr++;
    }
    return count;
}

// 将NMEA行添加到数据包缓冲区
void add_to_packet(const char *line) {
    if (line == NULL || strlen(line) < 6) return;

    pthread_mutex_lock(&data_mutex);
    size_t free_space = UDP_BUFFER_SIZE - strlen(packet_buffer) - 1;
    if (strlen(line) <= free_space) {
        strcat(packet_buffer, line);
    } else {
        printf("数据包缓冲区满，丢弃当前语句: %s", line);
    }
    pthread_mutex_unlock(&data_mutex);
}

// 发送数据包到服务器（UDP版本，带时间间隔控制）
void send_packet() {
    // 检查时间间隔
    if (!can_send_now()) {
        printf("未达到发送间隔(%d秒)，跳过本次发送\n", SEND_INTERVAL);
        return;
    }
    
    if (sockfd == -1) {
        if (!reconnect_server()) {
            printf("UDP socket创建失败，丢弃当前数据包\n");
            memset(packet_buffer, 0, UDP_BUFFER_SIZE);
            packet_in_progress = 0;
            current_packet_priority = PRIORITY_NONE;
            memset(current_packet_type, 0, sizeof(current_packet_type));
            return;
        }
    }

    // 使用sendto发送UDP数据包
    ssize_t send_len = sendto(sockfd, packet_buffer, strlen(packet_buffer), 0,
                             (struct sockaddr*)&server_addr, sizeof(server_addr));
    if (send_len < 0) {
        perror("发送UDP数据包失败");
        close(sockfd);
        sockfd = -1;
    } else {
        static int packet_count = 0;
        packet_count++;
        time_t now = time(NULL);
        struct tm *tm_info = localtime(&now);
        char time_str[20];
        strftime(time_str, sizeof(time_str), "%H:%M:%S", tm_info);
        
        printf("[%s] 第%d次发送UDP数据包，类型: %s，大小: %zu字节，目标: %s:%d，设备ID: %s，包含 %zu 条NMEA语句\n",
               time_str, packet_count, current_packet_type, strlen(packet_buffer), 
               server_ip, server_port, device_id, count_nmea_lines(packet_buffer));
        
        // 调试：打印前几行数据
        printf("数据包内容预览:\n");
        const char *ptr = packet_buffer;
        int line_count = 0;
        while (ptr != NULL && *ptr != '\0' && line_count < 3) {
            const char *line_end = strchr(ptr, '\n');
            if (line_end) {
                printf("  %.100s\n", ptr);
                ptr = line_end + 1;
                line_count++;
            } else {
                break;
            }
        }
        if (line_count > 0) {
            printf("  ...（共 %zu 条语句）\n", count_nmea_lines(packet_buffer));
        }
    }

    memset(packet_buffer, 0, UDP_BUFFER_SIZE);
    packet_in_progress = 0;
    current_packet_priority = PRIORITY_NONE;
    memset(current_packet_type, 0, sizeof(current_packet_type));
}

// 串口读取线程（RMC优先级高于GGA）
void *serial_read_thread(void *arg) {
    char buffer[SERIAL_BUFFER_SIZE];
    ssize_t bytes_read;
    char line_buffer[SERIAL_BUFFER_SIZE] = {0};
    int line_len = 0;
    int error_count = 0;

    while (running) {
        if (serial_fd == -1) {
            if (!reconnect_serial()) {
                printf("串口连接失败，将在5秒后再次尝试...\n");
                sleep(5);
                continue;
            }
        }

        bytes_read = read(serial_fd, buffer, SERIAL_BUFFER_SIZE - 1);
        
        if (bytes_read > 0) {
            error_count = 0;
            buffer[bytes_read] = '\0';
            
            for (int i = 0; i < bytes_read; i++) {
                if (buffer[i] == '\r') continue;
                
                line_buffer[line_len++] = buffer[i];
                
                if (buffer[i] == '\n' || line_len >= SERIAL_BUFFER_SIZE - 1) {
                    line_buffer[line_len] = '\0';
                    
                    if (strlen(line_buffer) > 10) {
                        // 检查是否是NMEA数据包开头（GGA或RMC）
                        if (is_nmea_start(line_buffer)) {
                            pthread_mutex_lock(&data_mutex);
                            
                            const char *new_packet_type = get_nmea_type(line_buffer);
                            int new_packet_priority = get_nmea_priority(line_buffer);
                            
                            // 判断是否需要发送当前数据包
                            int should_send_current = 0;
                            if (packet_in_progress && strlen(packet_buffer) > 0) {
                                if (new_packet_priority > current_packet_priority) {
                                    // 新数据包优先级更高，发送当前数据包
                                    printf("\n收到更高优先级的%s数据包（当前: %s），准备发送当前数据包...\n", 
                                           new_packet_type, current_packet_type);
                                    should_send_current = 1;
                                } else if (new_packet_priority == current_packet_priority) {
                                    // 相同优先级，发送当前数据包开始新的
                                    printf("\n收到新的%s数据包开头，准备发送上一个%s数据包...\n", 
                                           new_packet_type, current_packet_type);
                                    should_send_current = 1;
                                } else {
                                    // 新数据包优先级较低，忽略新数据包，继续收集当前数据包
                                    printf("\n收到较低优先级的%s数据包（当前: %s），忽略新数据包，继续收集当前数据包...\n", 
                                           new_packet_type, current_packet_type);
                                    pthread_mutex_unlock(&data_mutex);
                                    // 继续处理下一行
									add_to_packet(line_buffer);
                                    line_len = 0;
                                    memset(line_buffer, 0, SERIAL_BUFFER_SIZE);
                                    continue;
                                }
                            }
                            
                            if (should_send_current) {
                                pthread_mutex_unlock(&data_mutex);
                                send_packet();
                                pthread_mutex_lock(&data_mutex);
                            }
                            
                            // 开始新数据包
                            snprintf(packet_buffer, UDP_BUFFER_SIZE, "$ID:%s\r\n", device_id);
                            strncat(packet_buffer, line_buffer, 
                                    UDP_BUFFER_SIZE - strlen(packet_buffer) - 1);
                            packet_in_progress = 1;
                            current_packet_priority = new_packet_priority;
                            strncpy(current_packet_type, new_packet_type, sizeof(current_packet_type) - 1);
                            
                            printf("开始新数据包（%s开头，优先级: %d，ID: %s）: %s", 
                                   new_packet_type, new_packet_priority, device_id, line_buffer);
                            pthread_mutex_unlock(&data_mutex);
                        } else if (packet_in_progress) {
                            // 不是数据包开头，但正在收集数据包，添加到缓冲区
                            add_to_packet(line_buffer);
                        }
                    }
                    
                    line_len = 0;
                    memset(line_buffer, 0, SERIAL_BUFFER_SIZE);
                }
            }
        }
        else if (bytes_read == 0) {
            usleep(100000);
        }
        else {
            error_count++;
            if (error_count >= MAX_ERROR_COUNT) {
                if (errno != EAGAIN && errno != EWOULDBLOCK) {
                    perror("串口读取错误");
                    printf("连续错误次数: %d，尝试重新连接...\n", error_count);
                    close(serial_fd);
                    serial_fd = -1;
                    error_count = 0;
                }
            }
            usleep(100000);
        }
    }
    return NULL;
}

// 上传线程：处理超时未更新的数据包
void *upload_thread(void *arg) {
    while (running) {
        sleep(UPLOAD_INTERVAL);
        
        pthread_mutex_lock(&data_mutex);
        if (packet_in_progress && strlen(packet_buffer) > 0) {
            printf("\n超时未收到新NMEA数据包，准备发送当前缓存数据包（类型: %s，优先级: %d，ID: %s）...\n", 
                   current_packet_type, current_packet_priority, device_id);
            send_packet();
        }
        pthread_mutex_unlock(&data_mutex);
    }
    return NULL;
}

int main(int argc, char *argv[]) 
{
	
	SERIAL_PORT=argv[1];
    pthread_t serial_thread, upload_thread_id;

    signal(SIGINT, handle_signal);
    signal(SIGTERM, handle_signal);
    
    // 加载配置
    UPLOAD_INTERVAL = function("uci -q get nport.@ttyS2[0].hbpt");
    load_send_interval();
    load_server_info();
    
    printf("GNSS带ID数据包分割上传程序启动（UDP版本）- RMC优先级高于GGA\n");
    printf("配置: 串口=%s, 超时上传间隔=%d秒, 发送间隔=%d秒, 缓冲区大小=%d字节\n",
           SERIAL_PORT, UPLOAD_INTERVAL, SEND_INTERVAL, UDP_BUFFER_SIZE);
    printf("数据包优先级: RMC(%d) > GGA(%d) > 其他(%d)\n", 
           PRIORITY_RMC, PRIORITY_GGA, PRIORITY_NONE);
    printf("目标服务器: %s:%d\n", server_ip, server_port);
    printf("模式: 数据包格式=设备ID+分隔符+NMEA数据\n");
    printf("触发条件: 1.收到更高优先级数据包 2.收到同优先级新数据包 3.超时未更新 4.满足发送时间间隔\n");

    load_device_id();

    serial_fd = init_serial(SERIAL_PORT);
    if (serial_fd == -1) {
        printf("串口初始化失败，程序退出\n");
        return 1;
    }

    sockfd = init_udp_socket();
    if (sockfd == -1) {
        printf("UDP socket创建失败，将在后台尝试重连\n");
    }

    if (pthread_create(&serial_thread, NULL, serial_read_thread, NULL) != 0) {
        perror("创建串口读取线程失败");
        return 1;
    }

    if (pthread_create(&upload_thread_id, NULL, upload_thread, NULL) != 0) {
        perror("创建上传线程失败");
        pthread_cancel(serial_thread);
        return 1;
    }

    pthread_join(serial_thread, NULL);
    pthread_join(upload_thread_id, NULL);

    if (serial_fd != -1) {
        close(serial_fd);
    }
    if (sockfd != -1) {
        close(sockfd);
    }
    pthread_mutex_destroy(&data_mutex);

    printf("所有资源已释放，程序退出\n");
    return 0;
}