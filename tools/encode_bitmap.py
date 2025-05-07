#!/usr/bin/env python

import argparse
from PIL import Image


class RawMixin:
    def encode_byte(self, byte):
        self.append(byte)

    def encode_end(self):
        pass


class RleMixin:
    RLE_BYTE = 0
    RLE_SEQ = 1

    def __init__(self):
        self.state = self.RLE_BYTE
        self.count = 0
        self.prev_byte = None

    def eq_prev_byte(self, byte):
        if self.prev_byte is None:
            return False
        return byte == self.prev_byte

    def encode_byte(self, byte):
        if self.state == self.RLE_BYTE:
            self.append(byte)
            if self.eq_prev_byte(byte):
                self.state = self.RLE_SEQ
                self.count = 0
            else:
                self.prev_byte = byte
        elif self.state == self.RLE_SEQ:
            if self.eq_prev_byte(byte):
                self.count += 1
                if self.count == 255:
                    self.append(self.count)
                    self.prev_byte = None
                    self.state = self.RLE_BYTE
            else:
                self.append(self.count)
                self.append(byte)
                self.prev_byte = byte
                self.state = self.RLE_BYTE

    def encode_end(self):
        if self.state == self.RLE_SEQ:
            self.append(self.count)


class ImageEncoder:
    def __init__(self, size_format, orientation=0):
        self.size_format = size_format
        self.orientation = orientation
        self.bytes = []

    def append(self, value):
        self.bytes.append(value)
    
    def extend(self, values):
        self.bytes.extend(values)

    def append_size(self, width, height):
        if self.size_format == 2:
            self.extend([width % 256, width // 256, height % 256, height // 256])
        else:
            self.extend(width, height)

    @staticmethod
    def guess_format(image):
        if image.mode == "P":
            print("Indexed bitmap, will use RGBA")
            return "4/4/4/4"
        elif image.mode == "RGBA":
            return "4/4/4/4"
        else:
            return "5/6/5"

    def encode_1bit(self, image, rows):
        image = image.convert(mode='1')
        width, height = image.size
        self.append_size(width, height // rows)
        for y in range(0, height, 8):
            for x in range(width):
                value = 0
                for z in range(8):
                    if y + z < height:
                        if image.format == "XBM":
                            if image.getpixel((x, y + z)) > 0:
                                value += 1 << z
                        else:
                            if image.getpixel((x, y + z)) == 0:
                                value += 1 << z
                self.encode_byte(value)
        self.encode_end()
        return self.bytes

    def encode_4bits(self, image):
        image = image.convert(mode='L')
        width, height = image.size
        self.append_size(width, height)
        for y in range(0, height, 2):
            for x in range(width):
                value = 0xFF
                gray1 = self.get_pixel(image, x, y)
                if y + 1 < height:
                    gray2 = self.get_pixel(image, x, y + 1)
                else:
                    gray2 = 255
                for i in range(4):
                    if gray1 & (1 << (4 + i)):
                        value -= 1 << i
                    if gray2 & (1 << (4 + i)):
                        value -= 1 << (4 + i)
                self.encode_byte(value)
        self.encode_end()
        return self.bytes

    def encode_8bits(self, image):
        image = image.convert(mode='L')
        width, height = image.size
        self.append(0)
        self.append_size(width, height)
        if self.orientation == 270:
            for x in range(width):
                for y in range(height):
                    value = 0xFF - self.get_pixel(image, x, y)
                    self.encode_byte(value)
        else:
            for y in range(height):
                for x in range(width):
                    value = 0xFF - self.get_pixel(image, x, y)
                    self.encode_byte(value)
        self.encode_end()
        return self.bytes

    def encode_5_6_5(self, image):
        width, height = image.size
        self.append(0)
        self.append_size(width, height)
        for y in range(height):
            for x in range(width):
                pixel = self.get_pixel(image, x, y)
                # print(pixel)
                val = ((pixel[0] >> 3) << 11) + ((pixel[1] >> 2) << 5) + ((pixel[2] >> 3) << 0)
                self.encode_byte(val & 255)
                self.encode_byte(val >> 8)
        self.encode_end()
        return self.bytes

    def encode_4_4_4_4(self, image):
        width, height = image.size
        self.append(1)
        self.append_size(width, height)
        for y in range(height):
            for x in range(width):
                pixel = self.get_pixel(image, x, y)
                val = ((pixel[3] // 16) << 12) + ((pixel[0] // 16) << 8) + ((pixel[1] // 16) << 4) + ((pixel[2] // 16) << 0)
                self.encode_byte(val & 255)
                self.encode_byte(val >> 8)
        self.encode_end()
        return self.bytes

    def get_pixel(self, image, x, y):
        if self.orientation == 180:
            return image.getpixel((image.width - x - 1, image.height - y - 1))
        else:
            return image.getpixel((x, y))

    @staticmethod
    def create(size_format=1, orientation=0, encode_mixin=RawMixin):
        class ResultClass(ImageEncoder, encode_mixin):
            def __init__(self, *args, **kwargs):
                ImageEncoder.__init__(self, *args, **kwargs)
                encode_mixin.__init__(self)
        return ResultClass(size_format, orientation)


def main():
    parser = argparse.ArgumentParser(description='Bitmaps encoder')
    parser.add_argument('input', action="store", help="Input file name")
    parser.add_argument('output', action="store", help="Output file name")
    parser.add_argument('--format', action="store", help="Output format")
    parser.add_argument("--orientation", action="store", type=int, help="LCD orientation")
    parser.add_argument("--rle", help="Enable RLE compression", action="store_true")
    parser.add_argument("--rows", help="Image rows count (for 1bit format)", type=int, default=1)
    parser.add_argument("--size-format", help="Header image size format (1 or 2 bytes)", type=int, default=1)

    args = parser.parse_args()

    image = Image.open(args.input)
    encoder = ImageEncoder.create(args.size_format, args.orientation, RleMixin if args.rle else RawMixin)

    format = args.format
    if format == "auto":
        format = encoder.guess_format(image)
    if format == "1bit":
        bytes = encoder.encode_1bit(image, args.rows)
    elif format == "4bits":
        bytes = encoder.encode_4bits(image)
    elif format == "8bits":
        bytes = encoder.encode_8bits(image)
    elif format == "4/4/4/4":
        bytes = encoder.encode_4_4_4_4(image)
    elif format == "5/6/5":
        bytes = encoder.encode_5_6_5(image)

    with open(args.output, "w") as f:
        for byte in bytes:
            f.write("%d," % byte)


if __name__ == "__main__":
    main()
