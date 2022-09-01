using System;
using System.Text;
using System.Drawing;


namespace MatrixFontKezelo
{
    class MatrixFontKezelo
    {
        const int MaxMeretY = 8;
        const int MaxMeretX = 8;
        public int KarakterSzin { get; }
        public int ElvalasztoSzin { get; }
        public string EleresiUt { get; }
        public int FontMeretY { get; private set; }
        public int FontMeretX { get; private set; }
        public int ElvalasztoHossz { get; private set; }
        public int KarakterekDarab { get; private set; }
        private byte[,] karakterek;

        public MatrixFontKezelo(string eleresiUt) : this(Color.White, Color.Black, eleresiUt)
        { }
        public MatrixFontKezelo(Color elvalasztoSzin, Color karakterSzin, string eleresiUt)
        {
            KarakterSzin = karakterSzin.ToArgb();
            ElvalasztoSzin = elvalasztoSzin.ToArgb();
            EleresiUt = eleresiUt;
            Beolvas();
        }
        public bool Pixel(int karakterIndex, int x, int y)
        {
            int sor = karakterek[karakterIndex, y];
            return (sor & (128 >> x)) > 0;
        }
        private void Beolvas()
        {
            using (Bitmap bitmap = new Bitmap(EleresiUt))
            {
                BeolvasMeretek(bitmap);
                BeolvasKarakterek(bitmap);
            }
        }
        public void KeszitCHeaderFajl(string mappa, string nev)
        {
            StringBuilder sb = new StringBuilder();
            sb.AppendLine($"#ifndef {nev}_H");
            sb.AppendLine($"#define {nev}_H");
            sb.AppendLine();
            sb.AppendLine($"#define {nameof(KarakterekDarab)} {KarakterekDarab}");
            sb.AppendLine($"#define {nameof(FontMeretX)} {FontMeretX}");
            sb.AppendLine($"#define {nameof(FontMeretY)} {FontMeretY}");
            sb.AppendLine();

            sb.AppendLine($"const unsigned char {nev}[{nameof(KarakterekDarab)}][{nameof(FontMeretY)}]={{");
            for (int k = 0; k < KarakterekDarab; k++)
            {
                sb.AppendLine("{");
                for (int sor = 0; sor < FontMeretY; sor++)
                {
                    sb.AppendLine('B' + Convert.ToString(karakterek[k, sor], 2).PadLeft(8, '0') + ',');
                }
                sb.Remove(sb.Length - 3, 3);
                sb.AppendLine();
                sb.AppendLine("},");
            }
            sb.Remove(sb.Length - 3, 3);
            sb.AppendLine();
            sb.AppendLine("};");

            sb.AppendLine("#endif");
            System.IO.File.WriteAllText($"{mappa}\\{nev}.h", sb.ToString(), Encoding.UTF8);
        }

        private void BeolvasKarakterek(Bitmap bitmap)
        {
            KarakterekDarab = bitmap.Width / (FontMeretX + ElvalasztoHossz) + 2;
            karakterek = new byte[KarakterekDarab, FontMeretY];
            int k = 1;
            while (k < KarakterekDarab)
            {
                int y = 0;
                while (y < FontMeretY)
                {
                    int x = 0;
                    while (x < FontMeretX)
                    {
                        int pixel = bitmap.GetPixel((k - 1) * (ElvalasztoHossz + FontMeretX) + x, y).ToArgb();
                        if (pixel == KarakterSzin)
                        {
                            karakterek[k, y] += (byte)(128 >> x);
                        }
                        x++;
                    }
                    y++;
                }
                k++;
            }
        }
        public void MegjelenitKarakter(int karakterIndex)
        {
            Console.WriteLine($"{(char)karakterIndex} karakter: ");
            for (int y = 0; y < FontMeretY; y++)
            {
                for (int x = 0; x < FontMeretX; x++)
                {
                    Console.Write(Pixel(karakterIndex, x, y) ? "*" : " ");
                }
                Console.WriteLine();
            }
        }
        private void BeolvasMeretek(Bitmap bitmap)
        {
            FontMeretY = bitmap.Height;
            if (FontMeretY > MaxMeretY)
            {
                throw new ArgumentOutOfRangeException($"Font túl magas! Max méret: {MaxMeretY}. Aktuális méret: {FontMeretY}");
            }
            FontMeretX = 0;
            while (FontMeretX < bitmap.Width && ElvalasztoSzin != bitmap.GetPixel(FontMeretX, 0).ToArgb())
            {
                FontMeretX++;

            }
            if (FontMeretX > MaxMeretX)
            {
                throw new ArgumentOutOfRangeException($"Font túl széles! Max méret: {MaxMeretX}. Aktuális méret: {FontMeretX}");
            }

            while (FontMeretX + ElvalasztoHossz < bitmap.Width && ElvalasztoSzin == bitmap.GetPixel(FontMeretX + ElvalasztoHossz, 0).ToArgb())
            {
                ElvalasztoHossz++;
            }
        }
    }
}
