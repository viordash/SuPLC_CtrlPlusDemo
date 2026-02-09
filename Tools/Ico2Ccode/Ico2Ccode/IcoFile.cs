using System.Runtime.InteropServices;

namespace Ico2Ccode {
    public class IcoFile {

        public enum Type {
            TripleStateIcons,
            TwoStateIcons,
            SingleStateIcons
        }

        enum IcoType : UInt16 {
            ICO = 1,
            CUR = 2
        }

        [StructLayout(LayoutKind.Sequential)]
        struct Header {
            public UInt16 reserved;
            public IcoType type;
            public UInt16 count;
        }

        [StructLayout(LayoutKind.Sequential)]
        public struct Record {
            public byte width;
            public byte height;
            public byte colors;
            public UInt16 planes;
            public UInt16 bpp;
            public UInt32 size;
            public UInt32 offset;
        }

        static Header ReadHeader(ReadOnlySpan<byte> bytes) {
            if (bytes.Length < StructHelper.Size<Header>()) {
                throw new ArgumentException("ico file size is too small");
            }

            var header = StructHelper.FromBytes<Header>(bytes);
            if (header.type != IcoType.ICO) {
                throw new ArgumentException("incorrect ico type");
            }
            return header;
        }

        static Record ReadRecord(ReadOnlySpan<byte> bytes) {
            if (bytes.Length < StructHelper.Size<Record>()) {
                throw new ArgumentException("ico record is too small");
            }

            var record = StructHelper.FromBytes<Record>(bytes);
            if (record.colors != 2) {
                throw new ArgumentException("incorrect ico type");
            }
            if (record.planes != 0 && record.planes != 1) {
                throw new ArgumentException("incorrect planes");
            }
            if (record.size == 0) {
                throw new ArgumentException("ico record zero size");
            }
            return record;
        }

        public static List<Record> GetImageHeaders(ReadOnlySpan<byte> bytes) {
            var header = ReadHeader(bytes);
            if (header.count == 0) {
                throw new ArgumentException("zero images in a ico file");
            }

            var list = new List<Record>();
            var recordsData = bytes.Slice((int)StructHelper.Size<Header>());
            for (int i = 0; i < header.count; i++) {
                var record = ReadRecord(recordsData);
                recordsData = recordsData.Slice((int)StructHelper.Size<Record>());
                list.Add(record);
            }
            return list;
        }

        public static void ValidateSorting(IEnumerable<Record> imageRecords) {
            bool isSorted = imageRecords
                .Zip(imageRecords
                    .Skip(1), (a, b) => b.height >= a.height && b.width >= a.width)
                .All(x => x);
            if (!isSorted) {
                throw new ArgumentException("File has the wrong order in the icon set");
            }
        }

        public static List<BitmapFile.Dib> ExtractBitmaps(ReadOnlySpan<byte> bytes, IEnumerable<Record> imageRecords) {
            var list = new List<BitmapFile.Dib>();
            foreach (var record in imageRecords) {
                var bmp = bytes.Slice((int)record.offset);
                var roundedDib = BitmapFile.ExtractDib(bmp, record.size, record.width, record.height);
                var dib = BitmapFile.Convert2RawDib(roundedDib);
                list.Add(dib);
            }
            return list;
        }

        public static Type GetFileType(IEnumerable<Record> imageRecords) {
            var triples = imageRecords.Chunk(3);
            var triplesWithDiffSize = triples
                .Where(x => {
                    var first = x.First();
                    return x.Length != 3
                     || x
                        .Skip(1)
                        .Any(i => i.height != first.height || i.width != first.width);
                })
                ;
            var isThreeStateIcons = triples.Any() && !triplesWithDiffSize.Any();
            if (isThreeStateIcons) {
                return Type.TripleStateIcons;
            }

            var twins = imageRecords.Chunk(2);
            var twinsWithDiffSize = twins
                .Where(x => {
                    var first = x.First();
                    return x.Length != 2
                     || x
                        .Skip(1)
                        .Any(i => i.height != first.height || i.width != first.width);
                })
                ;
            var isTwoStateIcons = twins.Any() && !twinsWithDiffSize.Any();
            if (isTwoStateIcons) {
                return Type.TwoStateIcons;
            }
            return Type.SingleStateIcons;
        }

        public static List<CalibratedDib> CalibrateImage(Type fileType, List<BitmapFile.Dib> bitmaps) {
            switch (fileType) {
                case Type.TripleStateIcons: {
                        if (Enum.GetNames<IconCaliber>().Count() * 3 != bitmaps.Count) {
                            throw new ArgumentException("File has the incorrect count in icon set");
                        }
                        var calibrated = Enum.GetValues<IconCaliber>()
                            .SelectMany(x => new[] { x, x, x })
                            .Zip(bitmaps, (first, second) => new CalibratedDib() { Caliber = first, Dib = second })
                            .ToList();
                        return calibrated;
                    }

                case Type.TwoStateIcons: {
                        if (Enum.GetNames<IconCaliber>().Count() * 2 != bitmaps.Count) {
                            throw new ArgumentException("File has the incorrect count in icon set");
                        }
                        var calibrated = Enum.GetValues<IconCaliber>()
                            .SelectMany(x => new[] { x, x })
                            .Zip(bitmaps, (first, second) => new CalibratedDib() { Caliber = first, Dib = second })
                            .ToList();
                        return calibrated;
                    }

                case Type.SingleStateIcons: {
                        if (Enum.GetNames<IconCaliber>().Count() * 1 != bitmaps.Count) {
                            throw new ArgumentException("File has the incorrect count in icon set");
                        }
                        var calibrated = Enum.GetValues<IconCaliber>()
                            .Zip(bitmaps, (first, second) => new CalibratedDib() { Caliber = first, Dib = second })
                            .ToList();
                        return calibrated;
                    }
            }
            throw new ArgumentException("Incorrect file type");
        }
    }
}
