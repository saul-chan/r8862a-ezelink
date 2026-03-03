# !/usr/bin/python3
#
# Copyright (c) 2021-2024, Clourney Semiconductor. All rights reserved.
#
# This software and/or documentation is licensed by Clourney Semiconductor under limited terms and conditions.
# Reproduction and redistribution in binary or source form, with or without modification,
# for use solely in conjunction with a Clourney Semiconductor chipset, is permitted in condition which
# must retain the above copyright notice.
#
# By using this software and/or documentation, you agree to the limited terms and conditions.
#

import io
import sys
import csv
import socket
import struct

CLSCSI_PARSER_VER = 1
CLSCSI_REPORT_PORT = 37387


class ClscsiAttr:
    def __init__(self, name, attr_type, attr_fmt):
        self.name = name
        self.attr_type = attr_type
        self.attr_fmt = attr_fmt
        self.value = None


class ClscsiData:
    def __init__(self):
        self.reporter_ver = ClscsiAttr("reporter_ver", 0, "!B")
        self.ts_high = ClscsiAttr("ts_high", 1, "!I")
        self.ts_low = ClscsiAttr("ts_low", 2, "!I")
        self.dest_mac = ClscsiAttr("dest_mac", 3, "!6s")
        self.sta_mac = ClscsiAttr("sta_mac", 4, "!6s")
        self.rssi = ClscsiAttr("rssi", 5, "!b")
        self.rx_nss = ClscsiAttr("rx_nss", 6, "!B")
        self.tx_nss = ClscsiAttr("tx_nss", 7, "!B")
        self.channel = ClscsiAttr("channel", 8, "!B")
        self.bw = ClscsiAttr("bw", 9, "!B")
        self.n_subc = ClscsiAttr("n_subcarrier", 10, "!H")
        self.ppdu_fmt = ClscsiAttr("ppdu_fmt", 11, "!B")
        self.ltf_type = ClscsiAttr("ltf_type", 12, "!B")
        self.agc = ClscsiAttr("agc", 13, "!8s")
        self.ppdu_id = ClscsiAttr("ppdu_id", 14, "!H")
        self.iq_len = ClscsiAttr("iq_len", 15, "!H")
        self.iq_fmt = ClscsiAttr("iq_fmt", 16, "!B")
        self.iq_data = ClscsiAttr(
            "iq_data", 17, "!Xs"
        )  # data length is calculated runtime

    @staticmethod
    def set_attr_value(attr: ClscsiAttr, raw_data: str) -> None:
        """
        Set attribute value by raw_data.
        :param attr: attribute to set
        :param raw_data: unpacked raw data from socket
        :return: 0 on success or error code on errors
        """
        if attr.name in ["dest_mac", "sta_mac"]:
            attr.value = raw_data[0].hex(":")
        elif attr.name in ["agc", "iq_data"]:
            attr.value = raw_data[0].hex()
        else:
            attr.value = raw_data[0]
        if attr.name == "reporter_ver" and attr.value != CLSCSI_PARSER_VER:
            print(
                "Mismatched version between reporter (",
                attr.value,
                ") and parser (",
                CLSCSI_PARSER_VER,
                "). Skip."
            )
            return -1
        return 0


    def set_attr_by_tlv(self, attr_type: int, attr_len: int, val: str) -> int:
        """
        Store TLV from socket to attribute value.
        :param attr_type: Type of the attribute
        :param length: Length of the attribute
        :param val: Valute of the attribute
        :return: 0 on success or error code on errors
        """
        for key, attr in self.__dict__.items():
            if attr.attr_type != attr_type:
                continue
            if attr == self.iq_data:
                # build iq_data attr_fmt
                attr.attr_fmt = "!" + str(self.iq_len.value) + "s"

            # validate len
            fmt_len = struct.calcsize(attr.attr_fmt)
            if attr_len != fmt_len:
                print (
                    "Mismatched len for type=",
                    attr_type,
                     ", fmt_len=",
                    fmt_len,
                    ", frm_len=",
                    attr_len,
                )

            try:
                raw_data = struct.unpack(attr.attr_fmt, val)
            except struct.error as err:
                print("struct.error:\n" + str(err))
                return -1
            ret = self.set_attr_value(attr, raw_data)
            if ret != 0:
                return ret
        return 0


class ClscsiCsvFile:
    file_name = "clscsi_data.csv"

    def __init__(self, clscsi_data):
        self.col_title = []
        self.one_row_data = []
        self.clscsi_data = clscsi_data

        # build CSI column title
        for key, attr in clscsi_data.__dict__.items():
            self.col_title.append(attr.name)
        print(self.col_title)

        # write CSI column title to file
        with open(self.file_name, "w", newline="") as csvfile:
            csv_writer = csv.writer(csvfile)
            csv_writer.writerow(self.col_title)

    def save_data_row(self):
        """
        Save CSI data (info and I/Q data) to CSV file
        :return: None
        """
        self.one_row_data = []
        for key, attr in self.clscsi_data.__dict__.items():
            self.one_row_data.append(attr.value)

        # print CSI info (not including I/Q data) to terminal
        print(self.one_row_data[:-1])

        # init CSV data writer
        with open(self.file_name, "a", newline="") as csv_file:
            csv_writer = csv.writer(csv_file)
            # write full one row data, including I/Q data
            csv_writer.writerow(self.one_row_data)


def handle_data(csv_file: ClscsiCsvFile, data: str) -> int:
    """
    Handle data received from CLS-CSI reporter
    :param csv_file: the CSV file which CSI data will wrote to
    :param data: data Rxed from reporter
    :return: 0 on success or error code on error
    """
    pos = 0
    total_len = len(data)

    # parse TLV data: Type(1byte) + Length(4bytes) + Value(variable len)
    while pos < total_len - 5:
        attr_type, length = struct.unpack("!BI", data[pos : pos + 5])
        pos += 5
        if pos + length > total_len:
            print(
                "wrong data len=",
                length,
                " for type=",
                attr_type,
                " remain len=",
                total_len - pos,
            )
            return -1

        ret = csv_file.clscsi_data.set_attr_by_tlv(attr_type, length, data[pos : pos + length])
        if ret != 0:
            return ret
        pos += length

    # save CSI info and I/Q data to csv file
    csv_file.save_data_row()
    return 0


def main():
    try:
        clscsi_data = ClscsiData()
        clscsi_csv_file = ClscsiCsvFile(clscsi_data)

        s = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
        s.bind(("0.0.0.0", CLSCSI_REPORT_PORT))

        while True:
            data, addr = s.recvfrom(66560)
            # print('Rx data:', data.decode())
            # print('From address:', addr)

            handle_data(clscsi_csv_file, data)
    except Exception as err:
        print("\nError: \n" + str(err))
        sys.exit()


if __name__ == "__main__":
    main()
