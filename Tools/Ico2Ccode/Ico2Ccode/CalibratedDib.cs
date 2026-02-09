namespace Ico2Ccode {
    public enum IconCaliber {
        Small = 0,
        Medium = 1,
        Large = 2,
        XLarge = 3
    }

    public struct CalibratedDib {
        public BitmapFile.Dib Dib;
        public IconCaliber Caliber;
    }
}
