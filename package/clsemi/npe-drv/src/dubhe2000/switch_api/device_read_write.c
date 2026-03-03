//
// All device access from the Flexswitch API goes through these functions.
// They assume that the device is directly mapped into the address space
// but can easily be replaced to allow other methods of accessing the device.
//
#include "device_read_write.h"

// The address bit used to select between accumulator and default mode.
// Must correspond to the bit used in the apb2conf bridge. Note that
// this bit number is in the conf bus address space.
#define ACCUM_ADDR_BIT (19)

// Read one 32-bit word from the device.
// Parameters:
//   adapter: pointer to the board specific private data structure.
//   address: conf-bus index, i.e. address 0 is first 32-bit register and
//            address 1 is second 32-bit register.
//   mode: 0 - default
//             Reads the actual register and returns the value. If register is
//             wider than 32 bits it will return the part selected by the address.
//             The full register width will be stored in the accumulator
//             register.
//         1 - accumulator
//             Reads the value from the accumulator register. If register is
//             wider than 32 bits it will return the part selected by the
//             address.
uint32_t readFromDevice( struct dubhe1000_adapter *adapter,
                         uint64_t address,
                         int      mode) {
  uint32_t ret_data;

#if DUBHE1000_SWITCH_ACCUM_ADDR_ENABLE
  address |= ( mode << ACCUM_ADDR_BIT); // set the accumulator address bit depending on mode
#endif
  ret_data = readl(switch_base_addr + address * 4);

  return ret_data;
}

// Write one 32-bit word to the device.
// Parameters:
//   adapter: pointer to the board specific private data structure.
//   address: conf-bus index, i.e. address 0 is first 32-bit register and
//            address 1 is second 32-bit register.
//   mode: 0 - default
//             Writes to the the part of the accumulator register that is
//             selected by the address. Then the full width of the accumulator
//             register is written to the actual register.
//         1 - accumulator
//             Writes only to the the part of the accumulator register that is
//             selected by the address.
void writeToDevice(struct dubhe1000_adapter *adapter,uint64_t address,uint32_t data,int mode) {

#if DUBHE1000_SWITCH_ACCUM_ADDR_ENABLE
  address |= ( mode << ACCUM_ADDR_BIT); // set the accumulator address bit depending on mode
#endif
  writel(data, switch_base_addr + address * 4);
}
