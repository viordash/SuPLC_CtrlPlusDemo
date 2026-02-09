using System.Diagnostics;
using System.Text;

namespace Ico2Ccode {
    internal class Program {

        static void Main(string[] args) {
            if (args.Length != 3) {
                Console.WriteLine("Use: Ico2Ccode input.ico output.h (Small | Medium | Large | XLarge)");
                return;
            }

            var inputFilename = args[0];
            string outputFilename = args[1];
            if (Path.GetExtension(outputFilename) != ".h") {
                throw new ArgumentException("Output file must have the extension '.h'");
            }
            var allowedCaliber = Enum.Parse<IconCaliber>(args[2], ignoreCase: true);

            var bytes = File.ReadAllBytes(inputFilename);
            var imageRecords = IcoFile.GetImageHeaders(bytes);
            IcoFile.ValidateSorting(imageRecords);
            var fileType = IcoFile.GetFileType(imageRecords);
            var bitmaps = IcoFile.ExtractBitmaps(bytes, imageRecords);
            var calibrated = IcoFile.CalibrateImage(fileType, bitmaps);

            var cCode = BeginCCodeArray();

            for (int i = 0; i < calibrated.Count; i++) {
                var caliber = calibrated[i].Caliber;
                var dib = calibrated[i].Dib;
                if (caliber != allowedCaliber) {
                    continue;
                }

                FlipVertical(dib);
                NegativeColor(dib);
                var ssd1306Format = Convert2Ssd1306(dib);

                switch (fileType) {
                    case IcoFile.Type.TripleStateIcons: {
                            string nameSuffix;
                            switch (i % 3) {
                                case 0:
                                    nameSuffix = "_active";
                                    break;
                                case 1:
                                    nameSuffix = "_pressed";
                                    break;
                                default:
                                    nameSuffix = "_passive";
                                    break;
                            }
                            AddCCodeBitmapArray(cCode, dib, nameSuffix,
                                ssd1306Format);
                            break;
                        }
                    case IcoFile.Type.TwoStateIcons: {
                            var active = (i & 0x01) == 0;
                            AddCCodeBitmapArray(cCode, dib, active
                                ? "_active"
                                : "_passive",
                                ssd1306Format);
                            break;
                        }
                    case IcoFile.Type.SingleStateIcons:
                        AddCCodeBitmapArray(cCode, dib, "", ssd1306Format);
                        break;
                }

            }

            var code = EndCCodeArray(cCode);
            File.WriteAllText(outputFilename, code);
            Console.WriteLine($"File '{outputFilename}' ready");
        }

        static void FlipVertical(BitmapFile.Dib dib) {
            for (int row = 0; row < dib.Height / 2; row++) {
                for (int column = 0; column < dib.Width / 8; column++) {
                    var id0 = row * (dib.Width / 8) + column;
                    var id1 = (dib.Height - 1 - row) * (dib.Width / 8) + column;
                    var t = dib.Data[id0];
                    dib.Data[id0] = dib.Data[id1];
                    dib.Data[id1] = t;
                }
            }
        }

        static void NegativeColor(BitmapFile.Dib dib) {
            for (int row = 0; row < dib.Height; row++) {
                for (int column = 0; column < dib.Width / 8; column++) {
                    var id = row * (dib.Width / 8) + column;
                    dib.Data[id] = (byte)~dib.Data[id];
                }
            }
        }

        static byte[] Convert2Ssd1306(BitmapFile.Dib dib) {
            var fb = new byte[(dib.Width / 8) * dib.Height];
            for (int row = 0; row < dib.Height; row++) {
                for (int column = 0; column < (dib.Width / 8); column++) {
                    var xbm_offset = (row * (dib.Width / 8)) + column;
                    for (byte bit = 0; bit < 8; bit++) {
                        if ((dib.Data[xbm_offset] & 1 << (7 - bit)) != 0) {
                            fb[dib.Width * (row / 8) + column * 8 + bit] |= (byte)(1 << row % 8);
                        }
                    }
                }
            }
            return fb;
        }

        static string CreateCCodeBitmapArray(byte[] bytes) {
            var lines = new List<string>();
            for (int row = 0; row < (bytes.Length + 15) / 16; row++) {
                var colsCount = Math.Min(16, bytes.Length - row * 16);
                lines.Add(string.Join(", ", bytes
                    .Skip(row * 16)
                    .Take(colsCount)
                    .Select(x => "0x" + x.ToString("X2"))
                    ));
            }
            return string.Join(",\n", lines);
        }

        static StringBuilder BeginCCodeArray() {
            var sb = new StringBuilder();
            sb.AppendLine("#include \"Display/Common.h\"");
            return sb;
        }

        static void AddCCodeBitmapArray(StringBuilder sb, BitmapFile.Dib dib, string nameSuffix, byte[] ssd1306Format) {
            var cCodeArray = CreateCCodeBitmapArray(ssd1306Format);

            sb.AppendLine();
            sb.AppendLine($"const Bitmap icon{nameSuffix} = {{ //");
            sb.AppendLine($"    {{ {dib.Width},                                // width");
            sb.AppendLine($"      {dib.Height} }},                              // height");
            sb.AppendLine($"    {{ {cCodeArray} }}");
            sb.AppendLine("};");
        }

        static string EndCCodeArray(StringBuilder sb) {
            Debug.WriteLine(sb.ToString());
            return sb.ToString();
        }
    }
}