// See https://aka.ms/new-console-template for more information
Console.WriteLine("Hello, World!");

FrdReader.FrdResults results =  new FrdReader.FrdResults();
results.Read(@"C:\Users\tsvi\OneDrive - Det Kongelige Akademi\02_Resources\Software\PrePoMax\PrePoMax v2.0.0\Temp\Analysis-1.frd");

Console.ReadLine();