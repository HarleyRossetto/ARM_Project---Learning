using System;
using System.IO;
using static System.Console;
using System.Diagnostics;
using System.Collections.Generic;
using System.Linq;

namespace Size
{
    public struct SizeData
    {
        public UInt32 text {get; set;}
        public UInt32 data {get; set;}
        public UInt32 bss {get; set;}
        public UInt32 RAM {get => (data + bss);}
        public UInt32 FLASH {get => text;}

        public override string ToString() => $"*text {text} data {data} bss {bss}";
    }

    class Program
    {
        /*
        *   RAM:   [          ]   0.2% (used 4 bytes from 2048 bytes)
            Flash: [          ]   1.8% (used 586 bytes from 32256 bytes)
        *text    data     bss     dec     hex filename
          868    1076    8260   10204    27dc C:\Users\accou\Desktop\ARM_Project\bin\elf\arm-samc21n.elf
          data + bss is stored in ram
          *test = programme/flash(?)
        */

        static void Main(string[] args)
        {
            try {
            //If no arguments, return.
                if (!(args.Length > 0)) {
                    WriteLine("No Arguments");
                    return;
                }

                //Variable to store previous console foreground colour state
                ConsoleColor _storedColor = ForegroundColor;

                //Ensure file exists
                if (File.Exists(args[0])) {
                    UInt32 ramSize = Convert.ToUInt32(args[1]);
                    UInt32 flashSize = Convert.ToUInt32(args[2]);

                    //Create external process to run arm-none-eabi-size.exe
                    Process externalProcess = new Process {
                        StartInfo = new ProcessStartInfo {
                            FileName = "arm-none-eabi-size.exe",
                            Arguments = args[0],
                            RedirectStandardOutput = true
                        }
                    };
    
                    externalProcess.Start();
    
                    //Read the output from process
                    StreamReader reader = externalProcess.StandardOutput;
                    List<string> output = new List<string>(2);
                    while (!externalProcess.StandardOutput.EndOfStream)
                        output.Add(reader.ReadLine());
    
                    List<string> splitOutput = new List<String>(output[1].Split().Where(x => x != string.Empty));
                    SizeData szData = new SizeData();
                    szData.text = Convert.ToUInt32(splitOutput[0]);
                    szData.data = Convert.ToUInt32(splitOutput[1]);
                    szData.bss = Convert.ToUInt32(splitOutput[2]);   
    
                    Program.PrintUsageGraph("RAM", szData.RAM, ramSize);
                    Program.PrintUsageGraph("FLASH", szData.FLASH, flashSize);

                    WriteLine(output[0]);
                    ForegroundColor = ConsoleColor.Yellow;
                    WriteLine(output[1]);
                    ForegroundColor = _storedColor;
                } else {
                    _storedColor = ForegroundColor;
                    ForegroundColor = ConsoleColor.Yellow;
                    WriteLine("File {0} does not exist!", args[0]);
                    ForegroundColor = _storedColor;
                }
            } catch (Exception) {
                ConsoleColor temp = ForegroundColor;
                ForegroundColor = ConsoleColor.Red;
                WriteLine("Exception occured.");
                ForegroundColor = temp;
            }
        }

        public static void PrintUsageGraph(string name, uint usedBytes, uint totalBytes) {
                ConsoleColor _storedColor = ForegroundColor;
                Write("{0}:\t", name);
                if (BufferWidth > 81) {
                    Write("[");
                    Write(GenerateBarGraph(usedBytes, totalBytes));
                    ForegroundColor = _storedColor;
                    Write("] ");
                }
                ForegroundColor = ConsoleColor.Yellow;
                Write("{0,7:P2}", (float)usedBytes / totalBytes);
                ForegroundColor = _storedColor;
                Write(" (used ");
                ForegroundColor = ConsoleColor.Yellow;
                Write("{0,6}",usedBytes);
                ForegroundColor = _storedColor;
                Write(" bytes from ");
                ForegroundColor = ConsoleColor.Yellow;
                Write("{0,6}", totalBytes);
                ForegroundColor = _storedColor;
                WriteLine(" bytes)");
        }

        public static string GenerateBarGraph(UInt32 usedBytes, UInt32 totalBytes) {
            //Minimum buffer width for full display, 82
            //int bWidth = BufferWidth != null ? BufferWidth : 82;
            float percentage = (((float)usedBytes / totalBytes) * 100) / 4;
            if (percentage <= 15)
                ForegroundColor = ConsoleColor.Green;
            else if (percentage > 15 & percentage <=22)
                ForegroundColor = ConsoleColor.Yellow;
            else if (percentage > 22)
                ForegroundColor = ConsoleColor.Red;
            return string.Format("{0,-25}", new string('|', percentage <= totalBytes ? Convert.ToInt32(percentage) : 25));
        }
    }
}