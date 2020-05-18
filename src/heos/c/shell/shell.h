
#ifndef HDR__SHELL_SHELL_H__
#define HDR__SHELL_SHELL_H__

// --------------------------------------------------------------------------------------------------------------------
// values
// --------------------------------------------------------------------------------------------------------------------

typedef enum {
    shell_ValueType_string = 0,
    shell_ValueType_integer,
    //
    shell_maxValueType,
} shell_ValueType;

extern const char *shell_valueTypeNames[shell_maxValueType];

typedef struct {
    const char *displayName;
    shell_ValueType type;
    int storageOffset;
} shell_ValuePattern;

#define FIELD_OFFSET(s,f)   (((char*)&(((s*)1000000).f)) - (char*)1000000)

// --------------------------------------------------------------------------------------------------------------------
// options
// --------------------------------------------------------------------------------------------------------------------

typedef struct {
    const char *name;
    const char *documentation;
    int flagOffset; // type is char*, sets to 0 or 1 whether the option occurs; negative offset means no flag
        // (useful when an argument is used that indicates presence of the option already, so no flag is needed)
    shell_ValuePattern *argument; // nullable
} shell_OptionPattern;

// --------------------------------------------------------------------------------------------------------------------
// commands
// --------------------------------------------------------------------------------------------------------------------

typedef struct {

    // the name used to invoke the command
	const char *name;

	// options (nullable equivalent to empty; terminated by an entry with name = NULL)
	shell_OptionPattern *options;

	// mandatory positional arguments (nullable equivalent to empty; terminated by an entry with displayName = NULL)
	const shell_ValuePattern *fixedArguments;

	// repeated positional arguments (nullable; single entry since all repeated arguments must be alike)
	const shell_ValuePattern *repeatedArguments;

    // the actual command implementation
	void (*callback)(void);

} shell_CommandPattern;

// --------------------------------------------------------------------------------------------------------------------
// command implementation API
// --------------------------------------------------------------------------------------------------------------------

// Will use the shell_CommandPattern used to invoke the command. Returns 1 on success, 0 if the command line cannot
// be matched by the pattern. In the latter case, the command should just return immediately.
// The storage pointer can be NULL if the command does not accept any options or arguments.
//
// This function processes all options and arguments except repeated arguments, because there is no way to specify
// storage for an unbounded number of arguments. After calling this function once, use shell_processRepeatedArguments()
// for the repeated arguments.
int shell_processOptionsAndFixedArguments(void *storage);

// Can only be called after shell_processOptionsAndFixedArguments() has been called. Processes a single repeated
// argument, and stores its value in the designated storage field. Like shell_processOptionsAndFixedArguments(),
// returns 1 on success, 0 on syntax errors (this indicates that the command should just return immediately).
// Returns -1 to indicate that all repeated arguments have been consumed.
//
// As an alternative to a field in the storage block that changes its value for each call, the pointer passed to this
// function can point to the field to store the value in directly, such as a pointer to a local variable of the caller.
// In this case, set the offset in the argument pattern to 0 so this function does not add an offset.
int shell_processRepeatedArgument(void *storage);

// --------------------------------------------------------------------------------------------------------------------
// command invocation API
// --------------------------------------------------------------------------------------------------------------------

void shell_executeCommandLine(char *commandLine);

// --------------------------------------------------------------------------------------------------------------------
// command meta-data API
// --------------------------------------------------------------------------------------------------------------------

void shell_printSynopsis(const shell_CommandPattern *commandPattern);

shell_OptionPattern *shell_findShortOption(const shell_CommandPattern *commandPattern, char name);
shell_OptionPattern *shell_findOption(const shell_CommandPattern *commandPattern, const char *name);

#endif
