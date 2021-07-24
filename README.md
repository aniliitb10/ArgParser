A single-file, cross-platform and easy-to-use header-only library written in C++17 to parse command line arguments:

###### Example usage
```
#include <argParser/ArgParser.h>
#include <iostream>

int main(int argc, char* argv[])
{
    // The passed string becomes part of description when exeuted with -h or --help
    ArgParser argParser{"A demo example for argParser"};
    
    // true as the last argument makes it a mandatory argument
    argParser.addArgument("-l", "--logfile", "To get the log file", true);
    
    // as the function name suggests, 5 is the default wait_time
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

Following is the output when run with `-h` or `--help`
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
As this a single-file header-only library, just copy `argParser/ArgParser.h` in your project


###### Retrieving values:
```      
std::string logFile = argParser.retrieve("l").value()
// or argParser.retrieve("logFile").value()

const auto valueOpt = argParser.retrieve<int>(w).value();
// or argParser.retrieve<int>("wait_time").value()
if(valueOpt) // check if conversion succeeded
{
    const auto wait_time = valueOpt.value()
}
```
Note that `@retrieve` returns `std::optional<T>` where status of `std::optional` represents
- if the arg was configured and found command line options, and
- if the conversion from `std::string` to `T` was successful
So, the status MUST be checked before using the retrieved value

`@retrieve` has been specialized for: `std::string`, `bool` and `char`
- `std::string` -> no need to check the status of optional because conversion will always succeed
- `bool` -> if source value is `"true"`/`"false"`, then conversion succeeds
          otherwise conversion fails and returned value is `std::nullopt`
- `char` -> the conversion will succeed only if value has just one character 

There is another way to directly retrieve values (without `std::optional`) but it will throw if aything goes wrong, so it is better wrap such code blocks inside `try-catch` block
```
try
{
    const auto waitTime = argParser.retrieveMayThrow<int>("wait_time");
    const auto logfile = argParser.retrieveMayThrow("logfile");
}
catch (const std::runtime_error& e)
{
    std::cerr << "Error while extracting values: " << e.what() << "\n";
}
    
```

###### Help message:
If application is run with `-h` or `--help` flag, then `@needHelp` returns true and user should call `@helpMsg`
and must return because, in this case when `parse()` was called, no actual parsing was done
Otherwise, any attempt to retrieve any value will throw exception

###### Additional details:
- Functions which are not noexcept might throw exception
- arguments must only consist of alphanumeric characters and underscores
- short option must be shorter than the long option
- all parsed arguments are stored as `std::string`, so appropriate template type is needed to convert 

###### Dependencies
- No dependency for the header file, but it needs C++17 compiler
- Tests use `google-test` framework