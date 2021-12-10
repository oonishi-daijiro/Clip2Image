#include <iostream>
#include <atlimage.h>
#include <Windows.h>
#include <string>
#include <vector>
#if _MSC_VER > 1922 && !defined(_SILENCE_EXPERIMENTAL_FILESYSTEM_DEPRECATION_WARNING)
#define _SILENCE_EXPERIMENTAL_FILESYSTEM_DEPRECATION_WARNING
#endif
#include <experimental/filesystem>

using namespace std::experimental::filesystem;
using namespace std;

class Question
{
public:
  Question(string);
  string ask();
  void unexpected(string, bool);
  void unexpected();
  class option
  {
  public:
    option(string body, Question &p)
    {
      this->optionBody = body;
      this->p = &p;
    }
    void then(string);
    void then();

  private:
    string optionBody = "";
    string responseBody = "";
    Question *p;
  };
  option addOption(string);

private:
  struct optionAndExceptedAnswer
  {
    string option = "";
    string response = "";
  };
  bool skipOptionResponse = true;
  string questionBody;
  vector<optionAndExceptedAnswer> OptionsAndResponses;
  bool isLoop = false;
  string responseOfunexpectedAns = "";
  bool skipUnexpectedAns = true;
};

Question::Question(string questionBody)
{
  this->questionBody = questionBody;
};

Question::option Question::addOption(string body)
{
  Question::option option = Question::option(body, *this);
  return option;
}

void Question::option::then(string body)
{
  this->responseBody = body;
  Question::optionAndExceptedAnswer opsAndAns;
  opsAndAns.option = this->optionBody;
  opsAndAns.response = this->responseBody;
  this->p->skipOptionResponse = false;
  this->p->OptionsAndResponses.push_back(opsAndAns);
}

void Question::option::then()
{
  this->p->skipOptionResponse = true;
}

void Question::unexpected(string body, bool isLoop)
{
  this->responseOfunexpectedAns = body;
  this->isLoop = isLoop;
  this->skipUnexpectedAns = false;
}

void Question::unexpected()
{
  this->skipUnexpectedAns = true;
}

string Question::ask()
{
  std::cout << this->questionBody << std::endl;
  string answer;
  getline(cin, answer);
  for (const auto &e : this->OptionsAndResponses)
  {
    if (e.option == answer)
    {
      if (!this->skipOptionResponse)
      {
        std::cout << e.response << std::endl;
      }
      return answer;
    }
  }
  if (this->isLoop)
  {
    return this->ask();
  }
  if (!this->skipUnexpectedAns)
  {
    std::cout << this->responseOfunexpectedAns << std::endl;
  }
  return answer;
}

bool setBitmapFromClipboard(HANDLE &data)
{
  if (!IsClipboardFormatAvailable(CF_BITMAP))
    return false;
  OpenClipboard(NULL);
  GlobalLock(data);
  data = GetClipboardData(CF_BITMAP);
  CloseClipboard();
  return true;
}

string checkValidPath(string path)
{
  if (is_directory(path))
  {
    Question q = Question("This path is directory. Please set another path");
    string anotherPath = q.ask();
    return checkValidPath(anotherPath);
  }
  else if (exists(path))
  {
    Question q = Question("This file is already exists. Please input another name or whether overwrite[y/n]");
    q.addOption("y").then();
    q.addOption("n").then();
    q.unexpected();
    string ans = q.ask();
    if (ans == "y")
    {
      return path;
    }
    else if (ans == "n")
    {
      return "";
    }
    return checkValidPath(ans);
  }
  else if (path == "")
  {
    Question q = Question("Please input file name.");
    string fileName = q.ask();
    return checkValidPath(fileName);
  }
  return path;
}

void writeBitmapToFile(string path, HBITMAP &bitmapImage)
{
  CImage image;
  image.Attach(bitmapImage);
  CString imagePath = path.c_str();
  image.Save(imagePath);
  image.Detach();
  return;
}

int main(int argc, char *argv[])
{

  HGLOBAL data;
  bool isAvailable = setBitmapFromClipboard(data);
  if (!isAvailable)
  {
    std::cout << "There are no image data on the clipboard" << std::endl;
    return 0;
  }
  HBITMAP bitmapImage = (HBITMAP)data;
  string argFilename;
  if (argc <= 1)
  {
    Question q = Question("Please input file name");
    q.unexpected();
    argFilename = q.ask();
  }
  else
  {
    argFilename = argv[1];
  }
  argFilename = checkValidPath(argFilename);
  writeBitmapToFile(argFilename, bitmapImage);
  GlobalUnlock(data);
  return 0;
}
