
// ****************************************************************************
// File: Core.cpp
// Desc: Function String Associate plug-in
//
// ****************************************************************************
#include "stdafx.h"
#include <WaitBoxEx.h>

const UINT MAX_LINE_STR_COUNT = 10;
const UINT MAX_LABEL_STR = 60;  // Max size of each label
const int  MAX_COMMENT   = 764; // Max size of whole comment line

// Working label element info container
#pragma pack(push, 1)
struct STRC
{
	char str[MAX_LABEL_STR];
	int  refs;
};
#pragma pack(pop)

// === Function Prototypes ===
static void processFunction(func_t *pFunc);
static void filterWhitespace(LPSTR pszString);

// === Data ===
static ALIGN(16) STRC aString[MAX_LINE_STR_COUNT];
static UINT commentCount = 0;

// Main dialog
static const char mainDialog[] =
{
	"BUTTON YES* Continue\n" // "Continue" instead of "okay"
	"Function String Associate\n"

    #ifdef _DEBUG
    "** DEBUG BUILD **\n"
    #endif
	"Extracts strings from each function and intelligently adds them  \nto the function comment line.\n\n"
    "Version %Aby Sirmabus\n"
    "<#Click to open site.#www.macromonkey.com:k:1:1::>\n\n"

	" \n\n\n\n\n"
};


// Initialize
void CORE_Init()
{
    // String container should be align 16
    _ASSERT((sizeof(STRC) & 7) == 0);
}


// Un-initialize
void CORE_Exit()
{
}

static void idaapi doHyperlink(TView *fields[], int code) { open_url("http://www.macromonkey.com/bb/"); }

// Plug-in process
void CORE_Process(int iArg)
{
    try
    {
        char version[16];
        sprintf(version, "%u.%u", HIBYTE(MY_VERSION), LOBYTE(MY_VERSION));
        msg("\n>> Function String Associate: v: %s, built: %s, By Sirmabus\n", version, __DATE__);
        if (autoIsOk())
        {
            refreshUI();
            int iUIResult = AskUsingForm_c(mainDialog, version, doHyperlink);
            if (!iUIResult)
            {
                msg(" - Canceled -\n");
                return;
            }
            WaitBox::show();

            // Iterate through all functions..
            TIMESTAMP startTime = getTimeStamp();
            UINT functionCount = get_func_qty();
            char buffer[32];
            msg("Processing %s functions.\n", prettyNumberString(functionCount, buffer));
            refreshUI();

            for (UINT n = 0; n < functionCount; n++)
            {
                processFunction(getn_func(n));

                if (WaitBox::isUpdateTime())
                {
                    if (WaitBox::updateAndCancelCheck((int)(((float)n / (float)functionCount) * 100.0f)))
                    {
                        msg("* Aborted *\n");
                        break;
                    }
                }
            }

            WaitBox::hide();
            msg("Done: Generated %s string comments in %s.\n", prettyNumberString(commentCount, buffer), timeString(getTimeStamp() - startTime));
            msg("---------------------------------------------------------------------\n");
            refresh_idaview_anyway();
        }
        else
        {
            warning("Auto analysis must finish first before you run this plug-in!");
            msg("\n*** Aborted ***\n");
        }
    }
    CATCH()
}


// Remove whitespace & illegal chars from input string
static void filterWhitespace(LPSTR pstr)
{
	LPSTR ps = pstr;
	while(*ps)
	{
		// Replace unwanted chars with a space char
		char c = *ps;
		if((c < ' ') || (c > '~'))
			*ps = ' ';

		ps++;
	};

	// Trim any starting space(s)
	ps = pstr;
	while(*ps)
	{
		if(*ps == ' ')
	        memmove(ps, ps+1, strlen(ps));
		else
			break;
	};

	// Trim any trailing space
	ps = (pstr + (strlen(pstr) - 1));
	while(ps >= pstr)
	{
		if(*ps == ' ')
			*ps-- = 0;
		else
			break;
	};
}

static int __cdecl compare(const void *a, const void *b)
{
    STRC *sa = (STRC *)a;
    STRC *sb = (STRC *)b;
    return (sa->refs - sb->refs);
}

// Process function
static void processFunction(func_t *f)
{
    const int MIN_STR_SIZE = 4;

	// Skip tiny functions for speed
	if(f->size() >= 8)
	{
		// Skip if it already has type comment
		// TODO: Could have option to just skip comment if one already exists?
		BOOL skip = FALSE;
		LPSTR tempComment = get_func_cmt(f, true);
		if(!tempComment)
			get_func_cmt(f, false);

		if(tempComment)
		{
			// Ignore common auto-generated comments
            if (strncmp(tempComment, "Microsoft VisualC ", SIZESTR("Microsoft VisualC ")) != 0)
            {
                if (strstr(tempComment, "\ndoubtful name") == NULL)
                    skip = TRUE;
            }

            //if (skip)
            //    msg(EAFORMAT" c: \"%s\"\n", f->startEA, tempComment);
			qfree(tempComment);
		}

		// TODO: Add option to append to existing comments?

		if(!skip)
		{
			// Iterate function body looking for string references
            UINT nStr = 0;
            func_item_iterator_t it(f);

		    do
		    {
			    // Has an xref?
                ea_t currentEA = it.current();
			    xrefblk_t xb;
			    if(xb.first_from(currentEA, XREF_DATA))
			    {
				    // A string (ASCII, Unicode, etc.)?
				    flags_t flags = get_flags_novalue(xb.to);
                    if (isASCII(flags))
                    {
                        // Get the string
                        char buffer[MAX_LABEL_STR]; buffer[SIZESTR(buffer)] = 0;
                        int len = get_max_ascii_length(xb.to, ASCSTR_C, ALOPT_IGNHEADS);
                        if (len > (MIN_STR_SIZE + 1))
                        {
                            get_ascii_contents2(xb.to, len, ASCSTR_C, buffer, SIZESTR(buffer));
                            if (buffer[0])
                            {
                                // Clean it up
                                filterWhitespace(buffer);

                                // If it's not tiny continue
                                if (strlen(buffer) >= MIN_STR_SIZE)
                                {
                                    // If already in the list, just update it's ref count
                                    BOOL skip = FALSE;
                                    for (UINT j = 0; j < nStr; j++)
                                    {
                                        if (strcmp(aString[j].str, buffer) == 0)
                                        {
                                            aString[j].refs++;
                                            skip = TRUE;
                                            break;
                                        }
                                    }

                                    if (!skip)
                                    {
                                        // Add it to the list
                                        strcpy(aString[nStr].str, buffer);
                                        aString[nStr].refs = 1;
                                        ++nStr;

                                        // Bail out if we have max string count
                                        if (nStr >= MAX_LINE_STR_COUNT)
                                            break;
                                    }
                                }
                            }
                        }
				    }
			    }

		    }while(it.next_addr());

			// Got at least one string?
            if(nStr)
			{
				// Sort by reference count
                if (nStr > 1)
                    qsort(aString, nStr, sizeof(STRC), compare);

				// Concatenate a final comment string
                char comment[MAX_COMMENT + MAX_LABEL_STR] = { "STR: " };
                for (UINT i = 0; i < nStr; i++)
                {
                    STRC *sc = &aString[i];
                    int freeSize = ((MAX_COMMENT - strlen(comment)) - 1);
                    if ((freeSize > 6) && (freeSize < (int)(strlen(sc->str) + 2)))
                        break;
                    else
                    {
                        char temp[MAX_LABEL_STR]; temp[SIZESTR(temp)] = 0;
                        _snprintf(temp, SIZESTR(temp), "\"%s\"", sc->str);
                        strncat(comment, temp, freeSize);
                    }

                    // Continue line?
                    if ((i + 1) < nStr)
                    {
                        freeSize = ((MAX_COMMENT - strlen(comment)) - 1);
                        if (freeSize > 6)
                            strncat(comment, ", ", freeSize);
                        else
                            break;
                    }
                }

				// Add/replace comment
                //msg(EAFORMAT" %u\n", f->startEA, nStr);
				del_func_cmt(f, true); del_func_cmt(f, false);
				set_func_cmt(f, comment, true);
				commentCount++;
			}
		}
	}
}
