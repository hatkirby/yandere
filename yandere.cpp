#include <yaml-cpp/yaml.h>
#include <iostream>
#include <random>
#include <sstream>
#include <fstream>
#include <twitter.h>
#include <chrono>
#include <thread>
#include <algorithm>

template <class InputIterator>
std::string implode(InputIterator first, InputIterator last, std::string delimiter)
{
  std::stringstream result;
  
  for (InputIterator it = first; it != last; it++)
  {
    if (it != first)
    {
      result << delimiter;
    }
    
    result << *it;
  }
  
  return result.str();
}

template <class Container>
Container split(std::string input, std::string delimiter)
{
  Container result;
  
  while (!input.empty())
  {
    int divider = input.find(delimiter);
    if (divider == std::string::npos)
    {
      result.push_back(input);
      
      input = "";
    } else {
      result.push_back(input.substr(0, divider));
      
      input = input.substr(divider+delimiter.length());
    }
  }
  
  return result;
}

int main(int argc, char** argv)
{
  YAML::Node config = YAML::LoadFile("config.yml");
  
  twitter::auth auth;
  auth.setConsumerKey(config["consumer_key"].as<std::string>());
  auth.setConsumerSecret(config["consumer_secret"].as<std::string>());
  auth.setAccessKey(config["access_key"].as<std::string>());
  auth.setAccessSecret(config["access_secret"].as<std::string>());
  
  twitter::client client(auth);
  
  std::map<std::string, std::vector<std::string>> groups;
  std::ifstream datafile("data.txt");
  if (!datafile.is_open())
  {
    std::cout << "Could not find data.txt" << std::endl;
    return 1;
  }
  
  bool newgroup = true;
  std::string line;
  std::string curgroup;
  while (getline(datafile, line))
  {
    if (line.back() == '\r')
    {
      line.pop_back();
    }
    
    if (newgroup)
    {
      curgroup = line;
      newgroup = false;
    } else {
      if (line.empty())
      {
        newgroup = true;
      } else {
        groups[curgroup].push_back(line);
      }
    }
  }

  std::random_device random_device;
  std::mt19937 random_engine{random_device()};

  for (;;)
  {
    std::cout << "Generating tweet" << std::endl;
    std::string action = "{MAIN}";
    int tknloc;
    while ((tknloc = action.find("{")) != std::string::npos)
    {
      std::string token = action.substr(tknloc+1, action.find("}")-tknloc-1);
      std::string modifier;
      int modloc;
      if ((modloc = token.find(":")) != std::string::npos)
      {
        modifier = token.substr(modloc+1);
        token = token.substr(0, modloc);
      }
      
      int eqloc;
      std::string eqvarname;
      if ((eqloc = token.find("=")) != std::string::npos)
      {
        eqvarname = token.substr(0, eqloc);
        token = token.substr(eqloc+1);
      }
      
      std::string canontkn;
      std::transform(std::begin(token), std::end(token), std::back_inserter(canontkn), [] (char ch) {
        return std::toupper(ch);
      });
      
      std::string result;
      if (canontkn == "\\N")
      {
        result = "\n";
      } else {
        auto& group = groups[canontkn];
        std::uniform_int_distribution<int> dist(0, group.size() - 1);

        result = group[dist(random_engine)];
      }
      
      if (!eqvarname.empty())
      {
        groups[eqvarname].push_back(result);
      }
      
      if (modifier == "indefinite")
      {
        if ((result.length() > 1) && (isupper(result[0])) && (isupper(result[1])))
        {
          result = "an " + result;
        } else if ((result[0] == 'a') || (result[0] == 'e') || (result[0] == 'i') || (result[0] == 'o') || (result[0] == 'u'))
        {
          result = "an " + result;
        } else {
          result = "a " + result;
        }
      }
      
      std::string finalresult;
      if (islower(token[0]))
      {
        std::transform(std::begin(result), std::end(result), std::back_inserter(finalresult), [] (char ch) {
          return std::tolower(ch);
        });
      } else if (isupper(token[0]) && !isupper(token[1]))
      {
        auto words = split<std::list<std::string>>(result, " ");
        for (auto& word : words)
        {
          word[0] = std::toupper(word[0]);
        }
        
        finalresult = implode(std::begin(words), std::end(words), " ");
      } else {
        finalresult = result;
      }
      
      action.replace(tknloc, action.find("}")-tknloc+1, finalresult);
    }
    
    action.resize(140);
    
    try
    {
      client.updateStatus(action);
    } catch (const twitter::twitter_error& e)
    {
      std::cout << "Twitter error: " << e.what() << std::endl;
    }
    
    std::cout << action << std::endl;
    std::cout << "Waiting" << std::endl;
    
    std::this_thread::sleep_for(std::chrono::hours(1));
    
    std::cout << std::endl;
  }
}
