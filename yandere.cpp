#include <yaml-cpp/yaml.h>
#include <iostream>
#include <random>
#include <sstream>
#include <fstream>
#include <mastodonpp/mastodonpp.hpp>
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

void run(const std::string& configfile)
{
  YAML::Node config = YAML::LoadFile(configfile);

  mastodonpp::Instance instance{
    config["mastodon_instance"].as<std::string>(),
    config["mastodon_token"].as<std::string>()};
  mastodonpp::Connection connection{instance};

  std::map<std::string, std::vector<std::string>> groups;
  std::ifstream datafile(config["forms"].as<std::string>());
  if (!datafile.is_open())
  {
    std::cout << "Could not find forms file." << std::endl;
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
      } else if (line.substr(0, 2) == "{*") {
        std::string token = line.substr(2, line.find("}")-2);
        for (const std::string& other : groups[token]) {
          groups[curgroup].push_back(other);
          std::cout << curgroup << " *" << token << ": " << other << std::endl;
        }
      } else {
        groups[curgroup].push_back(line);
        std::cout << curgroup << ": " << line << std::endl;
      }
    }
  }

  std::random_device random_device;
  std::mt19937 random_engine{random_device()};

  for (;;)
  {
    std::cout << "Generating post" << std::endl;

    std::map<std::string, std::string> variables;

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
      } else if (variables.count(canontkn) == 1)
      {
        result = variables[canontkn];
      } else {
        auto& group = groups[canontkn];
        std::uniform_int_distribution<int> dist(0, group.size() - 1);

        result = group[dist(random_engine)];
      }

      if (!eqvarname.empty())
      {
        variables[eqvarname] = result;
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

    const mastodonpp::parametermap parameters{{"status", action}};
    auto answer{connection.post(mastodonpp::API::v1::statuses, parameters)};
    if (!answer)
    {
      if (answer.curl_error_code == 0)
      {
        std::cout << "HTTP status: " << answer.http_status << std::endl;
      }
      else
      {
        std::cout << "libcurl error " << std::to_string(answer.curl_error_code)
             << ": " << answer.error_message << std::endl;
      }
    }

    std::cout << action << std::endl;
    std::cout << "Waiting" << std::endl;

    std::this_thread::sleep_for(std::chrono::hours(2));

    std::cout << std::endl;
  }
}

int main(int argc, char** argv)
{
  if (argc != 2)
  {
    std::cout << "usage: yandere [configfile]" << std::endl;
    return -1;
  }

  std::string configfile(argv[1]);

  try
  {
    run(configfile);
  } catch (const mastodonpp::CURLException& e)
  {
    std::cout << e.what() << std::endl;
  }

  return 0;
}
