#include <boost/filesystem.hpp>
#include <iostream>
#include <vector>
#include <string>
#include <sstream>
#include <fstream>
#include <algorithm>

#include <windows.h>

std::string GetExePath() 
{
    char Buffer[160];
    GetModuleFileName(NULL, Buffer, 160);
    auto Pos = std::string(Buffer).find_last_of("\\/");
    return std::string(Buffer).substr(0, Pos);
}

namespace fs = boost::filesystem;

std::string VectorToStr(const std::vector<std::string> &Vec)
{
	std::string Ret;
	for(auto& Obj:Vec) Ret += Obj+' ';
	return Ret;
}

struct Adder
{
	std::string Name;
	std::vector<std::string> Lines;
	Adder(const std::string &_Name)
	{
		Name = _Name;
	}
	void PushLine(const std::string &NewLine)
	{
		Lines.push_back(NewLine);
	}
};

struct Generator
{
	std::vector<Adder> Adders;

	std::string MgenMainPath;
	
	std::string SrcPath;
	std::string ObjPath;
	std::string IncludeDir;
	std::string LibDir;
	std::string Libs;
	std::string OutPath;
	std::string CompilerFlags;
	bool IsMgen;
	
	std::string MakefileName;
	
	std::vector<std::string> Files;
	std::vector<std::string> Objs;
	std::vector<std::string> Cpps;
	
	std::vector<std::string> AllLines;
	
	Generator()
	{
		IsMgen = true;
		MakefileName = "makefile";
		OutPath = "run.exe";
		SrcPath = ".";
		ObjPath = ".";
	}
	
	void SetMgenToolPath(const std::string &Path)
	{
		MgenMainPath = Path;
		LoadAdders();
	}
	
	void LoadAdders()
	{
		std::ifstream File(MgenMainPath + "\\adders");
		// std::cout<<std::string(MgenMainPath + "\\adders");
		if(File.good())
		{
			std::string Line;
			while(File>>Line)
			{
				if(Line[0]=='+') Adders.push_back(Adder(Line.substr(1)));
				else if(Line[0]=='-') Adders.back().PushLine(Line);
			}
			File.close();		
		}
	}
	
	void ScriptLineNow(const std::string &Line)
	{		
		if(Line[0]=='-') AddFlag(Line[1], Line.substr(2));
	}
	
	void ScriptLine(const std::string &Text)
	{
		if(Text[0]=='-')
		{
			if(Text[1]=='c') AddFlag(Text[1], Text.substr(2));
			else AllLines.push_back(Text);
		}
		else if(Text=="help")
		{
			ShowHelp();
			exit(0);			
		}
		else
		{
			std::cout<<"> Error : Unknow instruction ["<<Text<<"]\n";
			exit(0);
		}
	}
	
	void ShowHelp()
	{
		std::cout<<"\nmgen path: "<<MgenMainPath<<"\n\n";
		std::ifstream File(MgenMainPath + "\\help");
		std::cout<<File.rdbuf();
		std::cout<<'\n';
	}
	
	void ScriptAllLines()
	{	
		for(auto& Line:AllLines) ScriptLineNow(Line);
	}
	
	void AddFlag(char FlagType, const std::string &Text)
	{
		if(FlagType=='S') 
		{
			SrcPath = Text;
			AddFlag('I', Text);
		}
		else if(FlagType=='O') ObjPath = Text;
		else if(FlagType=='I') IncludeDir += std::string("-I") + Text + ' ';
		else if(FlagType=='L') LibDir += std::string("-L") + Text + ' ';
		else if(FlagType=='l') Libs += std::string("-l") + Text + ' ';
		else if(FlagType=='a') OutPath = Text;
		else if(FlagType=='c') IsMgen = false; //Clear (no mgen file)
		else if(FlagType=='f') CompilerFlags += std::string("-") + Text + ' ';
		else if(FlagType=='v') //Variables
		{
			for(auto& Obj:Adders)
			{
				if(Obj.Name==Text)
				{
					for(auto& Line:Obj.Lines) ScriptLineNow(Line);
					return;
				}
			}
		}
		else
		{
			std::cout<<"> Error : Unknow flag [-"<<FlagType<<Text<<"]\n";
			exit(0);
		}
	}
	
	void LoadFile(const std::string &FilePath)
	{
		std::ifstream File(FilePath);
		if(File.good())
		{
			std::string Line;
			while(File>>Line)
			{	
				ScriptLine(Line);
			}
			File.close();
		}
	}
	void SetVectorFiles(const fs::path &Path)
	{
		if(fs::is_directory(Path))
		{
			for(auto&& Obj:fs::directory_iterator(Path))
			{
				if(fs::is_directory(Obj.path()))
					SetVectorFiles(Obj.path());
				else
				{
					auto Temp = Obj.path().string();
					auto SubStr = Temp.substr(Temp.find_last_of('.')+1);
					if(SubStr=="cpp")
					{
						std::replace(Temp.begin(), Temp.end(), '\\', '/');
						Temp = Temp.substr(0, Temp.find_last_of('.'));
						Files.push_back(Temp);
					}
				}
			}
		}
	}
	void LoadMgenFile()
	{
		std::ifstream File("mgen");
		if(File.good())
		{
			std::string Line;
			while(File>>Line)
			{
				ScriptLineNow(Line);
			}
			File.close();
		}
	}
	void GenerateMakefile()
	{		
		if(IsMgen) LoadMgenFile();
		ScriptAllLines();
		SetVectorFiles(SrcPath);
		
		//Objs
		for(int i=0; i<Files.size(); ++i) 
			Objs.push_back(ObjPath+"/O_"+std::to_string(i+1)+".o");
		
		//Cpps
		for(auto& Obj:Files)
			Cpps.push_back(Obj+".cpp");
			
		std::stringstream Buffer;
		
		Buffer<<"FILES = "<<VectorToStr(Files)<<'\n';
		Buffer<<"OBJ = "<<VectorToStr(Objs)<<"\n\n";
		
		// Buffer<<"# e.g. -ISFML/include\n";
		Buffer<<"IDIR = "<<IncludeDir<<'\n';
		// Buffer<<"# e.g. -LSFML/lib\n";
		Buffer<<"LDIR = "<<LibDir<<'\n';
		// Buffer<<"# e.g. -llua\n";
		Buffer<<"LIB = "<<Libs<<'\n';
		Buffer<<"CPP = "<<CompilerFlags<<"\n\n";
		
		Buffer<<"all: $(OBJ)\n";
		Buffer<<"\tg++ -o "<<OutPath<<" $^ $(LDIR) $(LIB)\n\n";
		
		for(int i=0; i<Objs.size(); ++i)
		{
			Buffer<<Objs[i]<<": "<<Cpps[i]<<'\n';
			Buffer<<"\tg++ -c $< -o $@ $(IDIR) $(CPP)\n\n";
		}

		if(ObjPath!=".") fs::create_directory(ObjPath);
		
		std::cout<<Buffer.str();
		std::cout<<"[ File: "<<MakefileName<<" | Out: "<<OutPath<<(!IsMgen ? " | No mgen file" : "")<<" ]\n";	
		
		std::ofstream File(MakefileName);
		File<<Buffer.str();
	}
};

int main(int argc, char** argv)
{
	// std::stringstream Buffer;
	
	// std::string FileName = "makefile";
	// // if(argc>1) FileName = std::string(argv[1]);
	// std::string IncDir = "Src";
	// if(argc>1) IncDir = std::string(argv[1]);
	// std::string ObjDir = "Obj";
	// if(argc>2) ObjDir = std::string(argv[2]);	
	
	// GenerateMakefileBuffer(Buffer, IncDir, ObjDir);	
	
	// Buffer<<"[ File: "<<FileName<<" ]";	
	// std::ofstream File(FileName);
	// std::cout<<Buffer.str();
	// File<<Buffer.str();
	// File.close();
	
	Generator Gen;
	std::string MgenMainPath = GetExePath();
	// std::cout<<"Main mgen path: "<<MgenMainPath<<'\n';
	Gen.SetMgenToolPath(MgenMainPath);
	if(argc>1) 
	{
		std::string Line;
		for(int i=1; i<argc; ++i)
		{
			Line = std::string(argv[i]);
			Gen.ScriptLine(Line);
		}
	}
	Gen.GenerateMakefile();
}
