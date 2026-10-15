#!/usr/bin/env python

import argparse
from PIL import Image, ImageOps


class RawMixin:
    def encode_byte(self, byte):
        self.append_byte(byte)

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
            self.append_byte(byte)
            if self.eq_prev_byte(byte):
                self.state = self.RLE_SEQ
                self.count = 0
            else:
                self.prev_byte = byte
        elif self.state == self.RLE_SEQ:
            if self.eq_prev_byte(byte):
                self.count += 1
                if self.count == 255:
                    self.append_byte(self.count)
                    self.prev_byte = None
                    self.state = self.RLE_BYTE
            else:
                self.append_byte(self.count)
                self.append_byte(byte)
                self.prev_byte = byte
                self.state = self.RLE_BYTE

    def encode_end(self):
        if self.state == self.RLE_SEQ:
            self.append_byte(self.count)


class ImageEncoder:
    def __init__(self, size_format, orientation=0):
        self.size_format = size_format
        self.orientation = orientation
        self.bytes = []

    def append_byte(self, value):
        self.bytes.append(value)
    
    def extend(self, values):
        self.bytes.extend(values)

    def append_short(self, value):
        self.append_byte(value % 256)
        self.append_byte(value // 256)

    def append_word(self, value):
        self.append_short(value % 65536)
        self.append_short(value // 65536)

    def append_format(self, value):
        self.append_word(value)

    def append_size(self, width, height):
        if self.size_format == 2:
            self.append_short(width)
            self.append_short(height)
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
        self.append_size(width, height)
        if self.orientation == 180:
            image = image.transpose(Image.ROTATE_180)
        elif self.orientation == 270:
            image = image.transpose(Image.ROTATE_270)
            image = image.transpose(Image.FLIP_LEFT_RIGHT)
            width, height = image.size
        for y in range(height):
            for x in range(width):
                value = 0xFF - image.getpixel((x, y))
                self.encode_byte(value)
        self.encode_end()
        return self.bytes

    def encode_color_part(self, value, bits):
        return value // pow(2, 8 - bits)

    def encode_16bits(self, value):
        self.encode_byte(value & 255)
        self.encode_byte(value >> 8)

    def encode_5_6_5(self, image):
        width, height = image.size
        self.append_format(0)
        self.append_size(width, height)
        if self.orientation == 180:
            image = image.transpose(Image.ROTATE_180)
        elif self.orientation == 270:
            image = image.transpose(Image.ROTATE_270)
            image = image.transpose(Image.FLIP_LEFT_RIGHT)
            width, height = image.size
        for y in range(height):
            for x in range(width):
                pixel = image.getpixel((x, y))
                val = (self.encode_color_part(pixel[0], 5) << 11) + (self.encode_color_part(pixel[1], 6) << 5) + (self.encode_color_part(pixel[2], 5) << 0)
                if val >= 256 * 256:
                    print(pixel, val)
                    exit(-1)
                self.encode_16bits(val)
        self.encode_end()
        return self.bytes

    def encode_4_4_4_4(self, image):
        width, height = image.size
        self.append_format(1)
        self.append_size(width, height)
        if self.orientation == 180:
            image = image.transpose(Image.ROTATE_180)
        elif self.orientation == 270:
            image = image.transpose(Image.ROTATE_270)
            image = image.transpose(Image.FLIP_LEFT_RIGHT)
            width, height = image.size
        for y in range(height):
            for x in range(width):
                pixel = image.getpixel((x, y))
                val = (self.encode_color_part(pixel[3], 4) << 12) + (self.encode_color_part(pixel[0], 4) << 8) + (self.encode_color_part(pixel[1], 4) << 4) + (self.encode_color_part(pixel[2], 4) << 0)
                self.encode_16bits(val)
        self.encode_end()
        return self.bytes

    def get_pixel(self, image, x, y):
        return image.getpixel((x, y))

    @staticmethod
    def create(size_format=1, orientation=0, encode_mixin=RawMixin):
        class ResultClass(ImageEncoder, encode_mixin):
            def __init__(self, *args, **kwargs):
                ImageEncoder.__init__(self, *args, **kwargs)
                encode_mixin.__init__(self)
        return ResultClass(size_format, orientation)


def write_cpp_definition(f, name, format, data):
    # constexpr definition of <name>Embedded, without the header (needs --size-format 2):
    # the object is in flash instead of RAM (no constructor called on startup)
    def write_values(values, value_format):
        for i in range(0, len(values), 16):
            f.write("  " + "".join(value_format % value for value in values[i:i + 16]) + "\n")

    if format in ("4/4/4/4", "5/6/5"):
        bitmap_format = data[0] + (data[1] << 8) + (data[2] << 16) + (data[3] << 24)
        width = data[4] + (data[5] << 8)
        height = data[6] + (data[7] << 8)
        pixels = [data[i] + (data[i + 1] << 8) for i in range(8, len(data), 2)]
        if len(pixels) != width * height:
            raise ValueError("%s: %d pixels instead of %d" % (name, len(pixels), width * height))
        f.write("static const pixel_t %sData[] __ALIGNED(4) = {\n" % name)
        write_values(pixels, "0x%04X,")
        f.write("};\n")
        f.write("constexpr Bitmap %sEmbedded(%d, %d, %d, %sData);\n" % (name, bitmap_format, width, height, name))
    elif format == "8bits":
        width = data[0] + (data[1] << 8)
        height = data[2] + (data[3] << 8)
        pixels = data[4:]
        if len(pixels) != width * height:
            raise ValueError("%s: %d pixels instead of %d" % (name, len(pixels), width * height))
        f.write("static const uint8_t %sData[] __ALIGNED(4) = {\n" % name)
        write_values(pixels, "%d,")
        f.write("};\n")
        f.write("constexpr Mask %sEmbedded(%d, %d, %sData);\n" % (name, width, height, name))
    else:
        raise ValueError("--cpp-name not supported with the %s format" % format)


def main():
    parser = argparse.ArgumentParser(description='Bitmaps encoder')
    parser.add_argument('input', action="store", help="Input file name")
    parser.add_argument('output', action="store", help="Output file name")
    parser.add_argument('--format', action="store", help="Output format")
    parser.add_argument("--orientation", action="store", type=int, help="LCD orientation")
    parser.add_argument("--rle", help="Enable RLE compression", action="store_true")
    parser.add_argument("--rows", help="Image rows count (for 1bit format)", type=int, default=1)
    parser.add_argument("--size-format", help="Header image size format (1 or 2 bytes)", type=int, default=1)
    parser.add_argument("--invert", help="Invert the bitmap (white vs black)", action="store_true")
    parser.add_argument("--cpp-name", help="Write a C++ constexpr definition (<name>Embedded) instead of the bytes")

    args = parser.parse_args()

    image = Image.open(args.input)
    if args.invert:
        image = ImageOps.invert(image.convert("L"))
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
        if args.cpp_name:
            write_cpp_definition(f, args.cpp_name, format, bytes)
        else:
            for byte in bytes:
                f.write("%d," % byte)


if __name__ == "__main__":
    main()
