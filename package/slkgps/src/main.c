#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <math.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <signal.h>

#define MAX_LINE_LENGTH 1024
#define BUFFER_SIZE 4096

// GPS数据结构
typedef struct {
    char mac[20];
    char id[30];
    char content[10];
    double lat;
    double lon;
    double alt;
    double speed;
    double dir;
    long stamp;
    int data_valid; // 数据有效性标志
} GPSData;

int running = 1; // 程序运行标志

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


// 信号处理函数，用于优雅退出
void signal_handler(int sig) {
    running = 0;
    printf("\n收到退出信号，正在关闭程序...\n");
}

// 将度分格式转换为十进制度
double dm_to_dd(double dm) {
    int degrees = (int)(dm / 100);
    double minutes = dm - degrees * 100;
    return degrees + minutes / 60.0;
}

// 解析NMEA时间日期为Unix时间戳
long parse_nmea_time(const char* time_str, const char* date_str) {
    if (strlen(time_str) < 6 || strlen(date_str) < 6) return 0;
    
    int hour, min, sec;
    double msec;
    sscanf(time_str, "%2d%2d%2d.%3lf", &hour, &min, &sec, &msec);
    
    int day, month, year;
    sscanf(date_str, "%2d%2d%2d", &day, &month, &year);
    year += 2000;
    
    struct tm timeinfo = {0};
    timeinfo.tm_year = year - 1900;
    timeinfo.tm_mon = month - 1;
    timeinfo.tm_mday = day;
    timeinfo.tm_hour = hour;
    timeinfo.tm_min = min;
    timeinfo.tm_sec = sec;
    
    return mktime(&timeinfo);
}

// 简单的NMEA校验和验证
int verify_checksum(const char* line) {
    char* asterisk = strchr(line, '*');
    if (asterisk == NULL) return 0;
    
    int calculated = 0;
    for (const char* p = line + 1; p < asterisk; p++) {
        calculated ^= *p;
    }
    
    int received;
    sscanf(asterisk + 1, "%2x", &received);
    
    return calculated == received;
}

// 解析GGA语句
int parse_gga(const char* line, GPSData* gps) {
    char* tokens[20];
    char copy[MAX_LINE_LENGTH];
    int token_count = 0;
    
    strcpy(copy, line);
    char* token = strtok(copy, ",");
    
    while (token != NULL && token_count < 20) {
        tokens[token_count++] = token;
        token = strtok(NULL, ",");
    }
    
    if (token_count < 10) return 0;
    
    // 定位质量指示
    if (strlen(tokens[6]) > 0 && atoi(tokens[6]) == 0) {
        return 0; // 无效定位
    }
    
    // 纬度
    if (strlen(tokens[2]) > 0 && strlen(tokens[3]) > 0) {
        double lat_dm = atof(tokens[2]);
        gps->lat = dm_to_dd(lat_dm);
        if (tokens[3][0] == 'S') gps->lat = -gps->lat;
    }
    
    // 经度
    if (strlen(tokens[4]) > 0 && strlen(tokens[5]) > 0) {
        double lon_dm = atof(tokens[4]);
        gps->lon = dm_to_dd(lon_dm);
        if (tokens[5][0] == 'W') gps->lon = -gps->lon;
    }
    
    // 海拔
    if (strlen(tokens[9]) > 0) {
        gps->alt = atof(tokens[9]);
    }
    
    return 1;
}

// 解析RMC语句
int parse_rmc(const char* line, GPSData* gps) {
    char* tokens[20];
    char copy[MAX_LINE_LENGTH];
    int token_count = 0;
    
    strcpy(copy, line);
    char* token = strtok(copy, ",");
    
    while (token != NULL && token_count < 20) {
        tokens[token_count++] = token;
        token = strtok(NULL, ",");
    }
    
    if (token_count < 10) return 0;
    
    // 状态检查
    if (tokens[2][0] != 'A') return 0;
    
    // 速度（节）
    if (strlen(tokens[7]) > 0) {
        gps->speed = atof(tokens[7]);
    }
    
    // 方向
    if (strlen(tokens[8]) > 0) {
        gps->dir = atof(tokens[8]);
    }
    
    // 时间戳
    if (strlen(tokens[1]) > 0 && strlen(tokens[9]) > 0) {
        gps->stamp = parse_nmea_time(tokens[1], tokens[9]);
    }
    
    return 1;
}

// 解析VTG语句
int parse_vtg(const char* line, GPSData* gps) {
    char* tokens[20];
    char copy[MAX_LINE_LENGTH];
    int token_count = 0;
    
    strcpy(copy, line);
    char* token = strtok(copy, ",");
    
    while (token != NULL && token_count < 20) {
        tokens[token_count++] = token;
        token = strtok(NULL, ",");
    }
    
    if (token_count < 8) return 0;
    
    // 方向
    if (strlen(tokens[1]) > 0) {
        gps->dir = atof(tokens[1]);
    }
    
    // 速度（节）
    if (strlen(tokens[5]) > 0) {
        gps->speed = atof(tokens[5]);
    }
    
    return 1;
}

// 将GPS数据格式化为JSON字符串
void format_gps_json(const GPSData* gps, char* json_buffer, size_t buffer_size) {
    snprintf(json_buffer, buffer_size,
        "{\n"
        "  \"mac\": \"%s\",\n"
        "  \"id\": \"%s\",\n"
        "  \"content\": \"%s\",\n"
        "  \"lat\": %.8f,\n"
        "  \"lon\": %.8f,\n"
        "  \"alt\": %.3f,\n"
        "  \"speed\": %.3f,\n"
        "  \"dir\": %.2f,\n"
        "  \"stamp\": %ld\n"
        "}",
        gps->mac, gps->id, gps->content, 
        gps->lat, gps->lon, gps->alt, 
        gps->speed, gps->dir, gps->stamp);
}

// 输出到标准输出和文件
void output_gps_data(const GPSData* gps) {
    char json_buffer[1024];
    format_gps_json(gps, json_buffer, sizeof(json_buffer));
    
    // 输出到标准输出
    printf("%s\n", json_buffer);
    fflush(stdout);
    
    // 以覆盖形式写入到 /tmp/gps 文件
    FILE* file = fopen("/tmp/gps", "w");
    if (file != NULL) {
        fprintf(file, "%s\n", json_buffer);
        fclose(file);
        printf("数据已写入 /tmp/gps 文件\n");
    } else {
        perror("无法打开 /tmp/gps 文件");
    }
}

// 处理单行NMEA数据
void process_nmea_line(const char* line, GPSData* gps) {
    // 验证校验和
    if (!verify_checksum(line)) {
        fprintf(stderr, "校验和错误: %s\n", line);
        return;
    }
    
    // 根据消息类型解析
    if (strncmp(line, "$GNGGA", 6) == 0 || strncmp(line, "$GPGGA", 6) == 0) {
        if (parse_gga(line, gps)) {
            gps->data_valid = 1;
        }
    } else if (strncmp(line, "$GNRMC", 6) == 0 || strncmp(line, "$GPRMC", 6) == 0) {
        if (parse_rmc(line, gps)) {
            gps->data_valid = 1;
        }
    } else if (strncmp(line, "$GNVTG", 6) == 0 || strncmp(line, "$GPVTG", 6) == 0) {
        parse_vtg(line, gps);
    }
}

// TCP客户端连接函数
int connect_to_gps(const char* server_ip, int port) {
    int sockfd;
    struct sockaddr_in server_addr;
    
    // 创建socket
    if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        perror("socket创建失败");
        return -1;
    }
    
    // 设置服务器地址
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(port);
    
    if (inet_pton(AF_INET, server_ip, &server_addr.sin_addr) <= 0) {
        perror("无效的IP地址");
        close(sockfd);
        return -1;
    }
    
    // 连接服务器
    if (connect(sockfd, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
        perror("连接失败");
        close(sockfd);
        return -1;
    }
    
    printf("已连接到GPS服务器 %s:%d\n", server_ip, port);
    return sockfd;
}

int main(int argc, char* argv[]) {
    // 设置信号处理
    signal(SIGINT, signal_handler);
    signal(SIGTERM, signal_handler);
    
    // 默认服务器地址和端口
	
    const char* server_ip = "127.0.0.1"; // 修改为你的GPS服务器IP
	int port = function("uci -q get nport.@ttyS2[0].localport");
    //int port = 2000; // 修改为你的GPS服务器端口
    
    // 从命令行参数获取服务器地址和端口
    if (argc >= 3) {
        server_ip = argv[1];
        port = atoi(argv[2]);
    }
    
    printf("正在连接GPS服务器 %s:%d...\n", server_ip, port);
    printf("输出文件: /tmp/gps (覆盖模式)\n");
    
    // 连接GPS服务器
    int sockfd = connect_to_gps(server_ip, port);
    if (sockfd < 0) {
        fprintf(stderr, "无法连接到GPS服务器\n");
        return 1;
    }
    
    GPSData gps = {0};
    //strcpy(gps.mac, "6c:71:bd:07:8f:90");
    //strcpy(gps.id, "EZE-TGW3120-54-00385");
	strcpy(gps.mac, order("fw_printsys | grep maclan0 | awk -F '=' '{print$2}'"));
	strcpy(gps.id, order("fw_printsys | grep sn | awk -F '=' '{print$2}'"));   
    strcpy(gps.content, "gps");
    
    char buffer[BUFFER_SIZE];
    char line[MAX_LINE_LENGTH];
    int line_pos = 0;
    int data_count = 0;
    
    printf("开始接收GPS数据...\n");
    printf("每收到一组完整数据将:\n");
    printf("1. 打印到标准输出\n");
    printf("2. 覆盖写入 /tmp/gps 文件\n\n");
    
    while (running) {
        // 接收数据
        ssize_t n = recv(sockfd, buffer, sizeof(buffer) - 1, 0);
        if (n <= 0) {
            if (running) {
                perror("接收数据错误或连接断开");
            }
            break;
        }
        
        buffer[n] = '\0';
        
        // 处理接收到的数据
        for (int i = 0; i < n && running; i++) {
            char c = buffer[i];
            
            if (c == '\n' || c == '\r') {
                if (line_pos > 0) {
                    line[line_pos] = '\0';
                    
                    // 处理NMEA语句
                    if (line[0] == '$') {
                        process_nmea_line(line, &gps);
                        
                        // 每收到一组完整数据就输出
                        if (gps.data_valid) {
                            data_count++;
                            printf("\n=== GPS数据 #%d ===\n", data_count);
                            output_gps_data(&gps);
                            gps.data_valid = 0; // 重置标志
                        }
                    }
                    
                    line_pos = 0;
                }
            } else if (line_pos < sizeof(line) - 1) {
                line[line_pos++] = c;
            }
        }
        
        // 限制输出频率，避免过于频繁
        usleep(100000); // 100ms
    }
    
    printf("\n已处理 %d 条GPS数据记录\n", data_count);
    close(sockfd);
    printf("程序退出\n");
    
    return 0;
}