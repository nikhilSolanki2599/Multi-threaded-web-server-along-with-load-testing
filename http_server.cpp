#include "http_server.hh"
#include <sys/types.h>
#include <vector>
#include <sys/dir.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <fstream>
#include <sstream>
#include <dirent.h>
#include <string.h>
#include <iostream>
#include <fstream>
vector<string> split(const string &s, char delim)
{
  vector<string> elems;

  stringstream ss(s);
  string item;

  while (getline(ss, item, delim))
  {
    if (!item.empty())
      elems.push_back(item);
  }

  return elems;
}

HTTP_Request::HTTP_Request(string request)
{
  vector<string> lines = split(request, '\n');
  vector<string> first_line = split(lines[0], ' ');

  this->HTTP_version = "1.0"; // We'll be using 1.0 irrespective of the request

  /*
   TODO : extract the request method and URL from first_line here
  */

  vector<string> met_and_u = split(lines[0], ' ');
  
  this->method = met_and_u[0];
  this->url = met_and_u[1];
  
  if (this->method != "GET")
  {
    cerr << "Method '" << this->method << "' not supported" << endl;
    exit(1);
  }
}

// HTTP_Response *handle_request(string req)
// {
//   HTTP_Request *request = new HTTP_Request(req);

//   HTTP_Response *response = new HTTP_Response();

//   string url = string("html_files") + request->url;

//   response->HTTP_version = "1.0";

//   struct stat sb;
//   struct dirent *dp;
//   DIR *dir;
//   string file_name;

  // if (stat(url.c_str(), &sb) == 0) // requested path exists
  // {
  //   response->status_code = "200";
  //   response->status_text = "OK";
  //   response->content_type = "text/html";

  //   string body;

  //   if (S_ISDIR(sb.st_mode)) {
  //     /*
  //     In this case, requested path is a directory.
  //     TODO : find the index.html file in that directory (modify the url
  //     accordingly)
  //     */
  //     if(url[url.size()-1] == '/')
  //       url += "index.html";
  //     else
  //       url += "/index.html";
  //   }
  //   const char *charUrl = url.c_str();
  //   /*
  //   TODO : open the file and read its contents
  //   */
  //   FILE *fileptr = fopen(charUrl, "r");
  //   struct stat si;
  //   stat(url.c_str(), &si);
  //   char buffer[si.st_size+1];
  //   int x = fread(buffer, si.st_size,1,fileptr);
  //   fclose(fileptr);
  //   /*
  //   TODO : set the remaining fields of response appropriately
  //   */
  //   response->body=buffer;
  //   response->content_length=to_string(si.st_size);
  // }

  // else
  // {
  //   response->status_code = "404";
  //   string line;
  //   response->status_text = "page not found";
  //   response->content_type = "text/html";
  //   line = "<html><head><title>404 Not found</title><body>ERROR 404 Page Not Found</head></body></html>";
  //   int len = line.length();
  //   response->body.append(line);
  //   response->content_length = to_string(len);
  //   /*
  //   TODO : set the remaining fields of response appropriately
  //   */
  // }

//   delete request;

//   return response;
// }

HTTP_Response *handle_request(string req)
{
  HTTP_Request *request = new HTTP_Request(req);

  HTTP_Response *response = new HTTP_Response();

  string url = string("html_files") + request->url;

  response->HTTP_version = "1.0";

  struct stat sb;
  struct dirent *dp;
  DIR *dir;
  string file_name;

  if (stat(url.c_str(), &sb) == 0) // requested path exists
  {
    response->status_code = "200";
    response->status_text = "OK";
    response->content_type = "text/html";

    string body, FILE;

    if (S_ISDIR(sb.st_mode))
    {
      /*
      In this case, requested path is a directory.
      TODO : find the index.html file in that directory (modify the url
      accordingly)
      */
      if ((dir = opendir(url.c_str())) == NULL)
      {
        perror("error while opening the directory");
      }
      else
      {
        while ((dp = readdir(dir)) != NULL)
        {
          file_name = dp->d_name;
          if (file_name == "index.html")
          {
            url = url + "/" + dp->d_name;
            break;
          }
        }
        if (file_name != "index.html")
        {
          url = "html_files/error.html";
        }
      }
      closedir(dir);
    }

    /*
    TODO : open the file and read its contents
    */
    string line;
    fstream my_file;
    my_file.open(url);
    int len = 0;
    while (getline(my_file, line))
    {
      len = len + line.length();
      response->body.append(line);
    }
    /*
    TODO : set the remaining fields of response appropriately
    */
    response->content_length = to_string(len);
    my_file.close();
  }

  else
  {
    response->status_code = "404";
    string line;
    response->status_text = "page not found";
    response->content_type = "text/html";
    line = "<html><head><title>404 Not found</title><body>ERROR 404 Page Not Found</head></body></html>";
    int len = line.length();
    response->body.append(line);
    response->content_length = to_string(len);
    /*
    TODO : set the remaining fields of response appropriately
    */
  }

  delete request;

  return response;
}

string HTTP_Response::get_string()
{
  /*
  TODO : implement this function
  */
  time_t now = time(0);
  struct tm tm = *gmtime(&now);
  char buf[1000];
  strftime(buf, sizeof buf, "%a, %d %b %Y %H:%M:%S %Z", &tm);
 
  string result = "";
  string HTTP_version = "HTTP/1.1 ";
  string status_code = this->status_code + " ";
  string status_text = this->status_text + "\r\n";
  string content_type = "Content-Type: " + this->content_type + "\r\n";
  string content_length = "Content-Length: " + this->content_length + "\r\n\n";
   result= HTTP_version + status_code + status_text + "Date : " + buf + "\r\n" + content_type + content_length+ body +"\n";
  //cout << "result:\n" + result << endl;
  return result;
}
