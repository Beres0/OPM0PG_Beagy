using System;
using System.Collections.Generic;
using System.IO;
using System.Linq;
using System.Runtime.CompilerServices;

namespace MatrixFontKezelo
{
    class Program
    {

        static DirectoryInfo ProjectMappa([CallerFilePath] string eleresiUt=null)
        {
            return Directory.GetParent(eleresiUt);
        }
        static DirectoryInfo LedMatrixSrcMappa()
        {
            return ProjectMappa().Parent.Parent.GetDirectories("Led_Matrix")[0].GetDirectories("src")[0];
        }
        static void Main(string[] args)
        {
            new MatrixFontKezelo
                (ProjectMappa() + "\\font127.png").KeszitCHeaderFajl(LedMatrixSrcMappa().FullName, "FontKeszlet");
        } 
    }
}
