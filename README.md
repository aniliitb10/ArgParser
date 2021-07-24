A cross-platform and easy-to-use header-only library written in C++17 to parse command line arguments:

###### Example usage
```
#include <argParser/ArgParser.h>
#include <iostream>

int main(int argc, char* argv[])
{
    ArgParser argParser{"A demo example for argParser"};
    argParser.addArgument("-l", "--logfile", "To get the log file", true); // true makes it a mandatory argument
    argParser.addArgumentWithDefault("-w", "--wait_time", "To get the wait time", 5);

    argParser.parse(argc, argv);
    if (argParser.needHelp())
    {
        std::cout << argParser.helpMsg() << "\n";
        return 0;
    }

    std::cout << "Log file name is: " << argParser.retrieve("logfile").value() << "\n";
    std::cout << "Wait time is: " << argParser.retrieve<int>("wait_time").value() << "\n";
}
```

Following is the output when run with -h or --help
```
~Projects/ArgParser$ ./argParser --help
A demo example for argParser
Following is a list of configured arguments:
-h, --help
        description: to get this message
-l, --logfile
        description: To get the log file, mandatory: true
-w, --wait_time
        description: To get the wait time, default: 5
```

As `[-l, --logfile]` was a mandatory argument, running the app without this argument will lead to crash:
```
~Projects/ArgParser$ ./argParser
terminate called after throwing an instance of 'std::runtime_error'
  what():  Couldn't find [-l, --logfile] mandatory argument in passed arguments
Aborted (core dumped)

```
However, app runs as expected when run with mandatory argument:
```
~Projects/ArgParser$ ./argParser -l=/var/tmp/app.log
Log file name is: /var/tmp/app.log
Wait time is: 5
```

###### Install
As this a single-file header-only library, just copy `argParser/ArgParser.h` in just your project


###### Retrieving values:
```      
std::string logFile = *argParser.retrieve("l") // or *argParser.retrieve("logFile")
const auto valueOpt = argParser.retrieve<int>(w); // or argParser.retrieve<int>("waitTime")
if(valueOpt) // check that conversion succeeded
{
    const auto waitTime = *valueOpt
}
```

@Note that @retrieve returns `std::optional<T>` where status of std::optional represents
- if the arg was configured and found command line options
- if the conversion from `std::string` to `T` was successful
So, the status MUST be checked before using the retrieved value

`@retrieve` has been specialized for: `std::string`, `bool` and `char`
`std::string` -> no need to check the status of optional because conversion status will always be true
`bool` -> if source string is `"true"`/`"false"`, then conversion succeeds
          otherwise conversion fails and returned value is std::nullopt

###### Help message:
If application is run with `-h` or `--help` flag, then `@needHelp` returns true and user should call @helpMsg
and must return because, in this case when parse() was called, no actual parsing was done
Otherwise, any attempt to retrieve any value will throw exception

###### Additional details:
- Functions which are not noexcept might throw exception
- arguments must only consist of alphanumeric characters and underscores
- short option must be shorter than the long option

###### Dependencies
- No dependency for the header file, but it needs C++17 compiler
- Tests use `google-test` framework