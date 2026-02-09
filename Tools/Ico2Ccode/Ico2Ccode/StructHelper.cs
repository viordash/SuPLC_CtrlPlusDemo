using System.Runtime.InteropServices;

namespace Ico2Ccode {
    public class StructHelper {
        public static uint Size<T>() where T : struct {
            return (uint)Marshal.SizeOf<T>();
        }

        public static uint Size<T>(T str) where T : struct {
            return (uint)Marshal.SizeOf<T>();
        }

        public static T FromBytes<T>(ReadOnlySpan<byte> data) where T : struct {
            return MemoryMarshal.Read<T>(data);
        }
    }
}
