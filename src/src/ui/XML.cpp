#include "ui/XML.h"

// Constructor
XML::XML(const char* fileName) {

    // Variable initialization
    this->tagParametersStart = -1;
    this->tagParametersEnd = -1;
    this->tagLabelsStart = -1;
    this->tagLabelsEnd = -1;
    this->tagMainStart = -1;
    this->tagMainEnd = -1;

    this->file = readFile(fileName);

    // Loop through the data to count tags

    // Primitive tags like '<' will get a single, translated, byte stored in TagType
    // Other tags will be stored as their string

    // Allocate a new string that has enough space for all tags of dynamic lengths
    // count primitive tags as +1, other tags as maybe +32

    // Then loop through the data again to populate this new string with values

    // This leaves a pre-processed string that will have quicker parsing due to the primitive tags
    // And this will also let you swap tags to anything that would fit in a 32 byte string

    return;

}

// Destructor
XML::~XML() {

    if (this->file != nullptr) delete this->file;
    if (this->tagSequence != nullptr) delete this->tagSequence;

    return;

}

// Instance Functions
void XML::setParameter(const char* tag, int value) {
    
    char* valueString = new char[32];
    intToString(value, valueString, 32);

    this->setParameter(tag, (const char*) valueString);

    delete[] valueString;

}

void XML::setParameter(const char* tag, float value) {

    char* valueString = new char[32];
    floatToString(value, valueString, 32, 4);

    this->setParameter(tag, (const char*) valueString);

    delete[] valueString;
    
}

void XML::setParameter(const char* tag, const char* value) {

    // Traverse the XML file, this can be sped up by:
    //      - Using the fact that every custom tag is 32 bytes, and every primitive tag is 1 byte
    //      - This lets you check each custom char until it mismatches, then jump almost directly to the start of the next

    // find any tags that match the requested one
    // Anything that matches will be swapped with value
    
}

WindowElement* XML::buildElement() {

}

Window* XML::buildWindow() {

}

void XML::locateReservedTags() {

    /*
        All the following variables are responsible for finding the substring in the file
        const char* -> string to find
        const int   -> length of substring (also tells when its been found) - Not counting the null terminator
        int         -> index of the current char being matched
    */


    // Struct to define a search string
    struct SearchTag {
        const char* string;     // String to find
        const int length;       // Length, also determines when its been found. Does NOT include the null terminator
        int index;              // Index of the current char being matched
        bool found;             // True if its been found already
    };


    // Parameters open
    SearchTag parametersStart = {"<parameters>", 12, 0, false};

    // Parameters close
    SearchTag parametersEnd = {"</parameters>", 13, 0, false};

    // Labels open
    SearchTag labelsStart = {"<labels>", 8, 0, false};

    // Labels close
    SearchTag labelsEnd = {"</labels>", 9, 0, false};

    // Main open
    SearchTag mainStart = {"<main>", 6, 0, false};

    // Main close
    SearchTag mainEnd = {"</main>", 7, 0, false};

    
    // Lambda helper function for the loop
    // This function updates the given SearchTag based on the character
    // In the context of the loop, this will be called for each character in the file
    auto matchChar = [](SearchTag& tag, char c) {

        if ( !tag.found ) {

            // Check against current
            if (c == tag.string[tag.index]) {

                tag.index++;

                // If the index matches the last index, the whole string has been found
                if (tag.index > tag.length)
                    tag.found = true;

            }

            // If no match, reset the index
            else tag.index = 0;

        }

    };


    // Stuff for the loop
    int index = 0;
    char currentChar;

    while (file[index] != '\0') {

        currentChar = file[index];


        // Parameters start
        matchChar(parametersStart, currentChar);

        // Parameters end
        matchChar(parametersEnd, currentChar);

        // Labels start
        matchChar(labelsStart, currentChar);

        // Labels end
        matchChar(labelsEnd, currentChar);

        // Main start
        matchChar(mainStart, currentChar);

        // Main end
        matchChar(mainEnd, currentChar);


        index++;

    }

    /*   Error checking stuff   */

    // Its not allowed to have EXACLTY ONE of these two tags, so I use XORs here

    // Parameters
    if ( (!parametersStart.found) ^ (!parametersEnd.found) ) {
        // Error
    }

    // Labels
    if ( (!labelsStart.found) ^ (!labelsEnd.found) ) {
        // Error
    }

    // There must be a main block, so if either doesnt exist, throw an error
    if ( (!mainStart.found) || (!mainEnd.found) ) {
        // Error
    }

}

int XML::getSequenceLength(char* file) {

    enum CurrentState {

        // No state active, between blocks
        IDLE,

        // Currently reading a tag
        READING_TAG

    };

    char currentChar;
    CurrentState currentState = IDLE;

    int length = 0;             // Return value
    int fullTagCount = 0;       // Counts the complex tags (XML::MAX_TAG_LENGTH bytes)
    int primitiveCount = 0;     // Counts the primitive tag buffers needed (3 bytes each)

    int index = 0;

    /* 
        Find "<main>". This is where the actual tokenization starts
    */

    const char* mainTag = "<main>";
    int mainTagIndex = 0;
    const int mainTagEndIndex = 7;

    while (file[index] != '\0') {

        currentChar = file[index];

        // If it matches, move to the next char
        if (currentChar == mainTag[mainTagIndex]) {

            mainTagIndex++;

            // If the index matches the last index, the whole string has been found
            if (mainTagIndex == mainTagEndIndex) {
                index++;
                break;
            }

        }

        // If not, reset the index
        else mainTagIndex = 0;

        index++;

    }

    // If this is true, the loop never found "<main>", so I log an error and return
    if (file[index] == '\0') {
        // Error
    }

    /*
        
    */

    // Start counting tags
    while (file[index] != '\0') {

        currentChar = file[index];

        if ( !(this->validChar(currentChar)) ) {
            index++;
            continue;
        } 

        switch (currentChar) {

            case '<': {

                if (currentState != IDLE) {
                    // Error
                }

                currentState = READING_TAG;

                break;

            }

            case '>': {

                if (currentState != READING_TAG) {
                    // Error
                }

                currentState = IDLE;

                break;

            }

            case '/':



            default: {

                if (currentState == IDLE) {
                    // Error
                }

                break;

            }

        }

        index++;

    }

    length = (fullTagCount * XML::MAX_TAG_LENGTH) + (primitiveCount * 3);

    return length;

}

void XML::populateTagSequence(char* file) {

}

bool XML::validChar(char c) {

    // Whitespace characters to track
    if (c == ' ') return true;
    if (c == '\n') return true;
    if (c == '\r') return true;

    // Allow only chars between a certain range, which is the range of readable ASCII chars
    // Between 0x20 and 0x7E inclusive
    return ( c >= 0x20 && c <= 0x7E );

}
