using Ico2Ccode;

namespace Ico2CcodeTests {
    public class IcoFileTests {
        [SetUp]
        public void Setup() {
        }


        [Test]
        public void GetFileType_isThreeStateIcons() {
            var imageRecords = new List<IcoFile.Record>() {
                new() { width = 16, height = 16 },
                new() { width = 16, height = 16 },
                new() { width = 16, height = 16 },
                new() { width = 24, height = 16 },
                new() { width = 24, height = 16 },
                new() { width = 24, height = 16 },
                new() { width = 24, height = 24 },
                new() { width = 24, height = 24 },
                new() { width = 24, height = 24 },
                new() { width = 32, height = 32 },
                new() { width = 32, height = 32 },
                new() { width = 32, height = 32 },
            };

            var fileType = IcoFile.GetFileType(imageRecords);
            Assert.That(fileType, Is.EqualTo(IcoFile.Type.TripleStateIcons));
        }

        [Test]
        public void GetFileType_TwoStateIcons() {
            var imageRecords = new List<IcoFile.Record>() {
                new() { width = 16, height = 16 },
                new() { width = 16, height = 16 },
                new() { width = 24, height = 16 },
                new() { width = 24, height = 16 },
                new() { width = 24, height = 24 },
                new() { width = 24, height = 24 },
                new() { width = 32, height = 32 },
                new() { width = 32, height = 32 },
            };

            var fileType = IcoFile.GetFileType(imageRecords);
            Assert.That(fileType, Is.EqualTo(IcoFile.Type.TwoStateIcons));
        }

        [Test]
        public void GetFileType_SingleStateIcons() {
            var imageRecords = new List<IcoFile.Record>() {
                new() { width = 16, height = 16 },
                new() { width = 16, height = 16 },
                new() { width = 24, height = 24 },
                new() { width = 24, height = 24 },
                new() { width = 32, height = 32 },
            };

            var fileType = IcoFile.GetFileType(imageRecords);
            Assert.That(fileType, Is.EqualTo(IcoFile.Type.SingleStateIcons));
        }

        [Test]
        public void ValidateSorting_With_TripleStateIcons() {
            var imageRecords = new List<IcoFile.Record>() {
                new() { width = 16, height = 16 },
                new() { width = 16, height = 16 },
                new() { width = 16, height = 16 },
                new() { width = 24, height = 16 },
                new() { width = 24, height = 16 },
                new() { width = 24, height = 16 },
                new() { width = 24, height = 24 },
                new() { width = 24, height = 24 },
                new() { width = 24, height = 24 },
            };
            IcoFile.ValidateSorting(imageRecords);
        }

        [Test]
        public void ValidateSorting_Throws_ArgumentException_When_TripleStateIcons_Not_Sorted() {
            var imageRecords = new List<IcoFile.Record>() {
                new() { width = 16, height = 16 },
                new() { width = 16, height = 16 },
                new() { width = 16, height = 16 },
                new() { width = 24, height = 24 },
                new() { width = 24, height = 24 },
                new() { width = 24, height = 24 },
                new() { width = 24, height = 16 },
                new() { width = 24, height = 16 },
                new() { width = 24, height = 16 },
            };
            Assert.Throws<ArgumentException>(() => IcoFile.ValidateSorting(imageRecords));
        }

        [Test]
        public void ValidateSorting_With_TwoStateIcons() {
            var imageRecords = new List<IcoFile.Record>() {
                new() { width = 16, height = 16 },
                new() { width = 16, height = 16 },
                new() { width = 24, height = 16 },
                new() { width = 24, height = 16 },
                new() { width = 24, height = 24 },
                new() { width = 24, height = 24 },
                new() { width = 32, height = 32 },
                new() { width = 32, height = 32 },
            };
            IcoFile.ValidateSorting(imageRecords);
        }

        [Test]
        public void ValidateSorting_Throws_ArgumentException_When_TwoStateIcons_Not_Sorted() {
            var imageRecords = new List<IcoFile.Record>() {
                new() { width = 16, height = 16 },
                new() { width = 16, height = 16 },
                new() { width = 24, height = 16 },
                new() { width = 24, height = 24 },
                new() { width = 24, height = 16 },
                new() { width = 24, height = 24 },
                new() { width = 32, height = 32 },
                new() { width = 32, height = 32 },
            };
            Assert.Throws<ArgumentException>(() => IcoFile.ValidateSorting(imageRecords));
        }
    }
}