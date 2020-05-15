
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

	// repeated positional arguments (nullable; single entry; storageOffset is ignored for these)
	const shellValuePattern *repeatedArgument;

    // the actual command implementation
	void (*callback)(void);

} shell_CommandPattern;

// --------------------------------------------------------------------------------------------------------------------
// command implementation API
// --------------------------------------------------------------------------------------------------------------------

// Will use the shell_CommandPattern used to invoke the command. Returns 1 on success, 0 if the command line cannot
// be matched by the pattern. In the latter case, the command should just return immediately.
// The storage pointer can be NULL if the command does not accept any options or arguments.
int shell_processOptionsAndArguments(void *storage);

// --------------------------------------------------------------------------------------------------------------------
// command invocation API
// --------------------------------------------------------------------------------------------------------------------

void shell_executeCommandLine(char *commandLine);
void shell_printSynopsis(const shell_CommandPattern *commandPattern);

#endif
