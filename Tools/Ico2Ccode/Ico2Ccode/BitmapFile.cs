using System.Runtime.InteropServices;

namespace Ico2Ccode {
    public class BitmapFile {
        public struct Dib {
            public byte[] Data;
            public int Width;
            public int Height;
        }

        [StructLayout(LayoutKind.Sequential, Pack = 2)]
        public struct BITMAPFILEHEADER {
            public ushort bfType;
            public uint bfSize;
            public ushort bfReserved1;
            public ushort bfReserved2;
            public uint bfOffBits;

            public BITMAPFILEHEADER() {
                bfSize = StructHelper.Size(this);
            }
        }

        public static Dib ExtractDib(ReadOnlySpan<byte> bytesBmp, uint imageSize, int width, int height) {
            if (!BITMAPINFO.TryParse(bytesBmp, out BITMAPINFO bitmapInfo)) {
                if (!BITMAPV5INFO.TryParse(bytesBmp, out BITMAPV5INFO bitmapV5Info)) {
                    throw new ArgumentException("bmp info parse error");
                }
                if (bitmapV5Info.bmiHeader.bV5BitCount != 1) {
                    throw new ArgumentException("incorrect bmp bit count");
                }
            } else {
                if (bitmapInfo.bmiHeader.biBitCount != 1) {
                    throw new ArgumentException("incorrect bmp bit count");
                }
            }
            var skipColorTables = StructHelper.Size<RGBQUAD>() * 2;
            var skip = bitmapInfo.bmiHeader.biSize + skipColorTables;
            return new Dib {
                Data = bytesBmp
                    .Slice((int)skip, (int)imageSize - (int)skip)
                    .ToArray(),
                Height = height,
                Width = width
            };
        }

        public static Dib Convert2RawDib(Dib dib) {
            var rawDib = new Dib {
                Data = new byte[dib.Width / 8 * dib.Height],
                Height = dib.Height,
                Width = dib.Width
            };

            var width_aligned = ((dib.Width + 31) / 32) * 32;
            for (int row = 0; row < dib.Height; row++) {
                for (int column = 0; column < dib.Width / 8; column++) {
                    var id = row * (dib.Width / 8) + column;
                    var idRounded = row * (width_aligned / 8) + column;
                    rawDib.Data[id] = dib.Data[idRounded];
                }
            }
            return rawDib;
        }
    }
}
