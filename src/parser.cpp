#include "parser.h"

#include <fstream>
#include <iomanip>
#include <iostream>
#include <sstream>

namespace mpegts
{
namespace
{
  // const uint8_t PTS_SYNC_BYTE = 0x47;

  // void read_pts_packet(std::ifstream &ifs)
  // {
  //   uint8_t sync_byte;
  //   ifs >> sync_byte;

  //   if (sync_byte != PTS_SYNC_BYTE)
  //   {
  //     std::stringstream ss;
  //     ss << std::setfill('0') << std::setw(sizeof(uint8_t) * 2) << std::hex << sync_byte;
  //     // log packet is incorrect
  //     return;
  //   }

  //   // https://en.wikipedia.org/wiki/MPEG_transport_stream#Layers_of_communication
  //   uint32_t header;
  //   ifs >> header;

  //   if (!(header & 0x800000))
  //   {
  //     // log packet is corrupt

  //     return;
  //   }

  //   if (header & 0xc0)
  //   {
		// 	// log transport scrumbling is unsupported
  //   	return;
  //   }

    // 6.
    // uint8_t adaptation_field_ctl = (header & 0x30); /*
                                                        // 01 – no adaptation field, payload only,
                                                        // 10 – adaptation field only, no payload,
                                                        // 11 – adaptation field followed by payload,
                                                        // 00 - RESERVED for future use [9
                                                    // */

    // if (!adaptation_field_ctl || adaptation_field_ctl == 2)
    // {
    // 	// log no payload;
    // 	return;
    // }


    // // TODO: check packet loss
    // // uint8_t continuity_counter = (header & 0xF);


    // // 2.

    // Set when a PES, PSI, or DVB-MIP packet begins immediately following the header.
    // bool pusi = (header & 0x400000);

    // // 4.
    // uint16_t pid = (header & 0x1fff00);


  // }
} // namespace

parser::parser(const std::string &file_name)
{
  // std::ifstream ifs;
  // auto exception_mask = ifs.exceptions() | std::ios::failbit;
  // ifs.exceptions(exception_mask);

  // try
  // {
  //   ifs.open(file_name, std::ios::in | std::ios::binary);
  // }
  // catch (const std::exception &)
  // {
  //   std::cerr << "Error: " << strerror(errno) << std::endl;
  // }

  std::cout << "opening file" << std::endl;
}

void parser::parse(data_received_callback callback)
{

}
} // namespace mpegts
