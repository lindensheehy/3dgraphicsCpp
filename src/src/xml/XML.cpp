#include "xml/XML.h"

// Constructor
XML::XML(UI* ui) {

    this->ui = ui;
    this->elementSet = new ElementSet();

    return;

}

// Destructor
XML::~XML() {

    if (this->elementSet != nullptr) delete this->elementSet;

    return;

}

// Instance functions
void XML::initDefaultElements() {

    /*   
        This just loads the signature for each element from "ui/WindowElement.h"
        Will need to change this function when adding a new default element
    */

    ParameterInfoBuilder* builder = new ParameterInfoBuilder();
    ParameterInfo* parameterInfo;

    /*   WindowDiv   */
    builder->addParameter("posx", TYPE_INT, 0);
    builder->addParameter("posy", TYPE_INT, 1);
    builder->addParameter("sizex", TYPE_INT, 2);
    builder->addParameter("sizey", TYPE_INT, 3);

    parameterInfo = builder->build();
    this->elementSet->addDefaultElement("div", parameterInfo, ElementSet::DIV);


    /*   WindowLine   */
    builder->addParameter("x1", TYPE_INT, 0);
    builder->addParameter("y1", TYPE_INT, 1);
    builder->addParameter("x2", TYPE_INT, 2);
    builder->addParameter("y2", TYPE_INT, 3);
    builder->addParameter("color", TYPE_INT, 4);

    parameterInfo = builder->build();
    this->elementSet->addDefaultElement("line", parameterInfo, ElementSet::LINE);


    /*   WindowFilledRect   */
    builder->addParameter("posx", TYPE_INT, 0);
    builder->addParameter("posy", TYPE_INT, 1);
    builder->addParameter("sizex", TYPE_INT, 2);
    builder->addParameter("sizey", TYPE_INT, 3);
    builder->addParameter("color", TYPE_INT, 4);

    parameterInfo = builder->build();
    this->elementSet->addDefaultElement("filledRect", parameterInfo, ElementSet::FILLED_RECT);


    /*   WindowOutlinedRect   */
    builder->addParameter("posx", TYPE_INT, 0);
    builder->addParameter("posy", TYPE_INT, 1);
    builder->addParameter("sizex", TYPE_INT, 2);
    builder->addParameter("sizey", TYPE_INT, 3);
    builder->addParameter("color", TYPE_INT, 4);

    parameterInfo = builder->build();
    this->elementSet->addDefaultElement("outlinedRect", parameterInfo, ElementSet::OUTLINED_RECT);


    /*   WindowCircle   */
    builder->addParameter("posx", TYPE_INT, 0);
    builder->addParameter("posy", TYPE_INT, 1);
    builder->addParameter("size", TYPE_INT, 2);
    builder->addParameter("color", TYPE_INT, 3);

    parameterInfo = builder->build();
    this->elementSet->addDefaultElement("circle", parameterInfo, ElementSet::CIRCLE);


    /*   WindowTextStatic   */
    builder->addParameter("posx", TYPE_INT, 0);
    builder->addParameter("posy", TYPE_INT, 1);
    builder->addParameter("text", TYPE_STRING, 2);

    parameterInfo = builder->build();
    this->elementSet->addDefaultElement("textStatic", parameterInfo, ElementSet::TEXT_STATIC);


    /*   WindowTextInput   */
    builder->addParameter("posx", TYPE_INT, 0);
    builder->addParameter("posy", TYPE_INT, 1);
    builder->addParameter("width", TYPE_INT, 2);
    builder->addParameter("id", TYPE_STRING, 3);

    parameterInfo = builder->build();
    this->elementSet->addDefaultElement("textInput", parameterInfo, ElementSet::TEXT_INPUT);


    /*   WindowTexture   */
    builder->addParameter("posx", TYPE_INT, 0);
    builder->addParameter("posy", TYPE_INT, 1);
    builder->addParameter("sizex", TYPE_INT, 2);
    builder->addParameter("sizey", TYPE_INT, 3);
    builder->addParameter("fileName", TYPE_STRING, 4);

    parameterInfo = builder->build();
    this->elementSet->addDefaultElement("texture", parameterInfo, ElementSet::TEXTURE);


    /*   WindowButton   */
    builder->addParameter("posx", TYPE_INT, 0);
    builder->addParameter("posy", TYPE_INT, 1);
    builder->addParameter("sizex", TYPE_INT, 2);
    builder->addParameter("sizey", TYPE_INT, 3);
    builder->addParameter("id", TYPE_STRING, 4);

    parameterInfo = builder->build();
    this->elementSet->addDefaultElement("button", parameterInfo, ElementSet::TEXTURE);


    /*   WindowDragable   */
    builder->addParameter("posx", TYPE_INT, 0);
    builder->addParameter("posy", TYPE_INT, 1);
    builder->addParameter("sizex", TYPE_INT, 2);
    builder->addParameter("sizey", TYPE_INT, 3);
    builder->addParameter("id", TYPE_STRING, 4);

    parameterInfo = builder->build();
    this->elementSet->addDefaultElement("dragable", parameterInfo, ElementSet::TEXTURE);


    // Clean up
    delete builder;

    return;

}

void XML::initCustomElements() {

}

WindowElement* XML::buildElement(XMLFile* xmlFile) {

    /*
        Yes this function is extremely long
        Yes a lot of that length comes from error logging
        Thats just the price of having specific error logging I suppose
    */


    // Helper function for error logging
    auto logError = [](const char* message, XMLFile* xmlFile, int index) -> void {

        logWrite("XMLFile::buildElement() ");
        logWrite(message, true);

        logWrite(" -> In file \"");
        logWrite(xmlFile->fileName);
        logWrite("\" at index ");
        logWrite(index, true);

        return;

    };


    // First ill make sure im not over the recursive limit
    if (this->callCount > CALL_LIMIT) {
        logError("hit its recursive call limit!", xmlFile, -1);
        logWrite(" -> This is likely due to circular dependancy. If this is not the case, go update XML::CALL_LIMIT", true);
        return nullptr;
    }

    this->callCount++;


    /*   Set up for the loop   */

    enum ParamState {
        LABEL,
        VALUE
    };

    struct VarContainer {

        // Data for handling the PARAMS section
        ParamState paramState = LABEL;
        ParameterType paramType = TYPE_NONE;
        int paramPosition = 0;

        // Buffers for holding PARAMS for default elements
        int* intBuffer;
        float* floatBuffer;
        char** stringBuffer;

        // Holder variables for each type. These might be put into the buffers above, or passed to XMLFile::setParameter
        int intTemp;
        float floatTemp;

        // Holds the name of the param for custom elements
        char* customParamName;

        // Global state of the PARAMS tag, will equal the last non NONE PARAMS tag
        PrimitiveTagState paramsTagState = CLOSE;

        // Holds the state of each prim tag in each iteration
        PrimitiveTagState elementTag;
        PrimitiveTagState paramsTag;
        PrimitiveTagState childrenTag;

        // Holds the string tag for this iteration
        char* stringTag;
        int stringTagLength;

        // Holds info about the current element being built
        ElementSet::Element* elementInfo;

        // Holds a copy of the XMLFile from elementInfo, that is if its a custom type
        XMLFile* elementXMLFile;

        // This is the hierarchy stack. opening an element pushes, closing pops
        LinkedList<WindowElement*>* stack = new LinkedList<WindowElement*>();

        // Flag says if the stack was emptied at any point
        bool poppedLast = false;

        // Holds the last popped element. This is also the return value
        WindowElement* lastPopped = nullptr;

        // This holds the current element that is being built
        WindowElement* currentElement = nullptr;

        // This is used to differentiate between a regular return and an error return
        bool error = false;

    };

    VarContainer vars;

    // Buffers for storing PARAMS values
    vars.intBuffer = new int[PARAMS_BUFFER_SIZE];
    vars.floatBuffer = new float[PARAMS_BUFFER_SIZE];
    vars.stringBuffer = new char*[PARAMS_BUFFER_SIZE];

    // Allocate and initialize the strings
    for (int i = 0; i < PARAMS_BUFFER_SIZE; i++) {
        vars.stringBuffer[i] = new char[MAX_TAG_LENGTH];
        memset(vars.stringBuffer[i], '\0', MAX_TAG_LENGTH);
    }

    logWrite("\n\n");

    /*   Loop through the tags   */
    
    for (int i = 0; i < xmlFile->main->stringTagCount; i++) {

        logWrite("Starting iteration at index ");
        logWrite(i);
        logWrite(":", true);

        // Get all the values I need
        vars.elementTag = xmlFile->main->getPrimitiveTag(i, ELEMENT);
        vars.paramsTag = xmlFile->main->getPrimitiveTag(i, PARAMS);
        vars.childrenTag = xmlFile->main->getPrimitiveTag(i, CHILDREN);

        vars.stringTag = xmlFile->main->getStringTag(i, &(vars.stringTagLength));

        logWrite("Successfully grabbed values from main tag sequence!", true);
        logWrite("String tag = \"");
        logWrite(vars.stringTag);
        logWrite("\"", true);

        /*
            Order is important here. 
            - First I check for PARAMS OPEN or PARAMS CLOSE
                - This is the simplest tag, as it just says how to handle the next string tags
            - Second I check if I am opening an element
                - In this case, I match it against the ElementSet to get the cooresponding Element object
                - This essentially just finds the context for how to treat the PARAMS section
            - Third I check if I should create an element
                - the following tags indicate this request:
                    - CHILDREN OPEN
                    - ELEMENT CLOSE_OPEN
                    - ELEMENT DOUBLE_CLOSE
                - If I can create an element, I add it as a child to the element at the top of the stack
            - Fourth I handle CHILDREN tags
                - CHILDREN OPEN pushes the last element to the stack
                - CHILDREN CLOSE pops the top element from the stack
            - Fifth I handle string tags in the params section
        */

        // Step 1: PARAMS tag handling
        switch (vars.paramsTag) {

            // Default value, do nothing
            case NONE:
                logWrite("Debug: PARAMS tag is NONE, no action taken", true);
                break;

            case OPEN: {
                logWrite("Debug: Entering PARAMS OPEN case", true);

                // This would happen if the last params section was not closed
                if (vars.paramsTagState == OPEN) {
                    logError("found a PARAMS OPEN tag while params are already open!", xmlFile, i);
                    vars.error = true;
                    logWrite("Debug: Error encountered, exiting early", true);
                    goto end;
                }

                vars.paramsTagState = OPEN;
                logWrite("Debug: PARAMS tag state set to OPEN", true);

                break;
            }

            case CLOSE: {
                logWrite("Debug: Entering PARAMS CLOSE case", true);

                // This would happen if there is no params section open
                if (vars.paramsTagState == CLOSE) {
                    logError("found a PARAMS CLOSE tag while none was open!", xmlFile, i);
                    vars.error = true;
                    logWrite("Debug: Error encountered, exiting early", true);
                    goto end;
                }

                vars.paramsTagState = CLOSE;
                logWrite("Debug: PARAMS tag state set to CLOSE", true);

                break;
            }

            // This should never happen
            default:
                logWrite("Debug: Unexpected PARAMS tag encountered", true);
                break;
        }

        // Step 2: Create element if possible
        logWrite("Debug: Checking if an element should be created", true);

        if (vars.childrenTag == OPEN || vars.elementTag == CLOSE_OPEN || vars.elementTag == DOUBLE_CLOSE) {
            
            if (vars.elementInfo == nullptr) {
                logError("Tried to create en element before finding element context", xmlFile, i);
                vars.error = true;
                goto end;
            }

            logWrite("Debug: Element creation conditions met, proceeding", true);

            // Can't create an element while the PARAMS section is open
            if (vars.paramsTagState == OPEN) {
                logError("is trying to create an element before closing the parameters section!", xmlFile, i);
                vars.error = true;
                logWrite("Debug: Exiting early due to error", true);
                goto end;
            }

            logWrite("Debug: Determining element type", true);

            switch (vars.elementInfo->type) {

                // Default elements defined in code
                case ElementSet::DIV:
                    logWrite("Debug: Creating WindowDiv element", true);
                    vars.currentElement = new WindowDiv(vars.intBuffer[0], vars.intBuffer[1], vars.intBuffer[2], vars.intBuffer[3]);
                    break;

                case ElementSet::LINE:
                    logWrite("Debug: Creating WindowLine element", true);
                    vars.currentElement = new WindowLine(vars.intBuffer[0], vars.intBuffer[1], vars.intBuffer[2], vars.intBuffer[3], vars.intBuffer[4]);
                    break;

                case ElementSet::FILLED_RECT:
                    logWrite("Debug: Creating WindowFilledRect element", true);
                    logWrite("Using params (");
                    logWrite(vars.intBuffer[0]);
                    logWrite(", ");
                    logWrite(vars.intBuffer[1]);
                    logWrite(", ");
                    logWrite(vars.intBuffer[2]);
                    logWrite(", ");
                    logWrite(vars.intBuffer[3]);
                    logWrite(", ");
                    logWrite(vars.intBuffer[4]);
                    logWrite(")", true);
                    vars.currentElement = new WindowFilledRect(vars.intBuffer[0], vars.intBuffer[1], vars.intBuffer[2], vars.intBuffer[3], vars.intBuffer[4]);
                    break;

                case ElementSet::OUTLINED_RECT:
                    logWrite("Debug: Creating WindowOutlinedRect element", true);
                    vars.currentElement = new WindowOutlinedRect(vars.intBuffer[0], vars.intBuffer[1], vars.intBuffer[2], vars.intBuffer[3], vars.intBuffer[4]);
                    break;

                case ElementSet::CIRCLE:
                    logWrite("Debug: Creating WindowCircle element", true);
                    vars.currentElement = new WindowCircle(vars.intBuffer[0], vars.intBuffer[1], vars.intBuffer[2], vars.intBuffer[3]);
                    break;

                case ElementSet::TEXT_STATIC:
                    logWrite("Debug: Creating WindowTextStatic element", true);
                    vars.currentElement = new WindowTextStatic(vars.intBuffer[0], vars.intBuffer[1], vars.stringBuffer[2]);
                    break;

                case ElementSet::TEXT_INPUT:
                    logWrite("Debug: Creating WindowTextInput element", true);
                    vars.currentElement = new WindowTextInput(vars.intBuffer[0], vars.intBuffer[1], vars.intBuffer[2], vars.stringBuffer[3]);
                    break;

                case ElementSet::TEXTURE:
                    logWrite("Debug: Creating WindowTexture element", true);
                    vars.currentElement = new WindowTexture(vars.intBuffer[0], vars.intBuffer[1], vars.intBuffer[2], vars.intBuffer[3], vars.stringBuffer[4]);
                    break;

                case ElementSet::BUTTON:
                    logWrite("Debug: Creating WindowButton element", true);
                    vars.currentElement = new WindowButton(vars.intBuffer[0], vars.intBuffer[1], vars.intBuffer[2], vars.intBuffer[3], vars.stringBuffer[4]);
                    break;

                case ElementSet::DRAGABLE:
                    logWrite("Debug: Creating WindowDragable element", true);
                    vars.currentElement = new WindowDragable(vars.intBuffer[0], vars.intBuffer[1], vars.intBuffer[2], vars.intBuffer[3], vars.stringBuffer[4]);
                    break;

                // This is for elements defined in other XML files
                case ElementSet::CUSTOM:
                    logWrite("Debug: Handling CUSTOM element type", true);

                    // Try building the element from the corresponding XMLFile
                    vars.currentElement = this->buildElement(vars.elementXMLFile);
                    logWrite("Debug: CUSTOM element built successfully", true);

                    // Free the XMLFile that was used to create this element
                    delete vars.elementXMLFile;
                    logWrite("Debug: Deleted temporary XMLFile for CUSTOM element", true);

                    break;

                // Should never happen
                default:
                    logWrite("Debug: Warning! Reached unexpected default case in element creation", true);
                    break;
            }

            // Add this element as a child to the element at the top of the stack
            // If length == 0, this element is the parent, so do nothing for now
            if (vars.stack->length > 0) {
                logWrite("Debug: Adding new element as a child to the last element in the stack", true);
                vars.stack->getLast()->addChild(vars.currentElement);
            } else {
                logWrite("Debug: No parent element, this is the root element", true);
            }
        }

        // Step 3: Opening a new element
        logWrite("Debug: Checking if a new element should be opened", true);

        if (vars.elementTag == OPEN || vars.elementTag == CLOSE_OPEN) {
            logWrite("Debug: Element tag is OPEN or CLOSE_OPEN, proceeding", true);

            // Can't open an element inside the PARAMS section
            if (vars.paramsTagState == OPEN) {
                logError("wants to open an element inside the parameters section!", xmlFile, i);
                logWrite("Debug: Error encountered -> This is likely a typo in the file", true);
                vars.error = true;
                logWrite("Debug: Exiting early due to error", true);
                goto end;
            }

            logWrite("Debug: Attempting to match element name", true);
            vars.elementInfo = this->elementSet->matchElement(vars.stringTag);

            if (vars.elementInfo == nullptr) {
                logError("found an unrecognized element name!", xmlFile, i);
                vars.error = true;
                logWrite("Debug: Exiting early due to unrecognized element", true);
                goto end;
            }

            logWrite("Debug: Element recognized, checking type", true);

            // Make a copy of the inner XMLFile object if it's a CUSTOM type
            if (vars.elementInfo->type == ElementSet::CUSTOM) {
                logWrite("Debug: Element type is CUSTOM, copying XMLFile", true);
                vars.elementXMLFile = vars.elementInfo->xmlFile->copy();
            } else {
                logWrite("Debug: Element type is not CUSTOM, setting XMLFile to nullptr", true);
                vars.elementXMLFile = nullptr;
            }
        }

        // Step 4: CHILDREN tag handling
        logWrite("Debug: Handling CHILDREN tag", true);

        switch (vars.childrenTag) { 

            // Default value, do nothing
            case NONE:
                logWrite("Debug: CHILDREN tag is NONE, no action taken", true);
                break;

            case OPEN: {
                logWrite("Debug: CHILDREN tag is OPEN", true);

                // If currentElement == nullptr, there's a problem in the XML file
                if (vars.currentElement == nullptr) {
                    logError("is trying to open a children section before having a valid element!", xmlFile, i);
                    logWrite(" -> This usually happens if the opening tag for the element has a mistake", true);
                    vars.error = true;
                    logWrite("Debug: Exiting early due to error", true);
                    goto end;
                }

                // No special handling here, this just always pushes to the stack
                logWrite("Debug: Pushing currentElement onto the stack", true);
                vars.stack->pushBack(vars.currentElement);

                break;
            }

            case CLOSE: {
                logWrite("Debug: CHILDREN tag is CLOSE", true);

                /*
                    Note: 
                    This code assumes that the closing tags will always match up properly.
                    If any don’t, the code will still run, but you may get unexpected results.
                    Best way to think of this is by treating all closing tags </tag> the same.
                */

                // Pop the last item on the stack and store for further use
                logWrite("Debug: Popping the last element from the stack", true);
                vars.lastPopped = vars.stack->popBack();

                // Check the flag to see if the last element has already been popped
                if (vars.poppedLast) {
                    logError("found more than one parent element in main!", xmlFile, i);
                    logWrite(" -> There should be exactly one parent element holding everything in the <main> tag", true);
                    vars.error = true;
                    logWrite("Debug: Exiting early due to error", true);
                    goto end;
                }

                // If the length is now 0, the stack has been emptied so set the flag
                if (vars.stack->length == 0) {
                    logWrite("Debug: Stack is empty, marking poppedLast as true", true);
                    vars.poppedLast = true;
                }

                break;
            }

            // This should never happen
            default:
                logWrite("Debug: Unexpected CHILDREN tag value encountered", true);
                break;
        }

        // Step 5: Handle string tags in the PARAMS section
        logWrite("Debug: Entering PARAMS section handling", true);

        if (vars.paramsTagState == OPEN) {
            logWrite("Debug: PARAMS tag is OPEN", true);

            if (vars.elementInfo == nullptr) {
                logError("is trying to parse the PARAMS section before matching an element!", xmlFile, i);
                vars.error = true;
                logWrite("Debug: Exiting early due to error", true);
                goto end;
            }

            switch (vars.paramState) {
                case LABEL: {
                    logWrite("Debug: PARAMS state is LABEL", true);

                    // Different handling for custom elements PARAMS
                    if (vars.elementInfo->type == ElementSet::CUSTOM) {
                        logWrite("Debug: Handling custom element PARAMS, storing name", true);
                        vars.customParamName = vars.stringTag;
                        break;
                    }

                    logWrite("Checking for nulls...", true);

                    if (vars.elementInfo == nullptr) {
                        logWrite("vars.elementInfo is nullptr!!!", true);
                    }

                    if (vars.elementInfo->params == nullptr) {
                        logWrite("vars.elementInfo->params is nullptr!!!", true);
                    }

                    logWrite("No nulls found!", true);

                    logWrite("Debug: Matching parameter name", true);
                    vars.paramType = vars.elementInfo->params->matchParameter(vars.stringTag, &(vars.paramPosition));

                    if (vars.paramType == TYPE_NONE) {
                        logError("failed to match a parameter name!", xmlFile, i);
                        vars.error = true;
                        logWrite("Debug: Exiting early due to error", true);
                        goto end;
                    }

                    logWrite("Matched the parameter!", true);

                    vars.paramState = VALUE;
                    break;
                }

                case VALUE: {
                    logWrite("Debug: PARAMS state is VALUE", true);

                    // Different handling for custom elements PARAMS
                    if (vars.elementInfo->type == ElementSet::CUSTOM) {
                        logWrite("Debug: Custom element detected, skipping default handling", true);
                        break;
                    }

                    // Early return if attempting to access an out-of-range index
                    if (vars.elementInfo->type != ElementSet::CUSTOM && vars.paramPosition >= PARAMS_BUFFER_SIZE) {
                        logError("wants to put a parameter at a position outside the buffer range!", xmlFile, i);
                        logWrite(" -> Either update XML::initDefaultElements() or XML::PARAMS_BUFFER_SIZE", true);
                        vars.error = true;
                        logWrite("Debug: Exiting early due to error", true);
                        goto end;
                    }

                    logWrite("Debug: Processing parameter type", true);
                    switch (vars.paramType) {
                        case TYPE_INT: {
                            logWrite("Debug: Casting string to INT", true);

                            // Try decimal conversion
                            if (!stringToInt(vars.stringTag, &(vars.intTemp), MAX_TAG_LENGTH)) {
                                logWrite("Debug: Decimal conversion failed, attempting hex", true);

                                // Try hexadecimal conversion
                                if (!stringHexToInt(vars.stringTag, &(vars.intTemp), MAX_TAG_LENGTH)) {
                                    logError("wants to cast a string into an int but can't!", xmlFile, i);
                                    logWrite(" -> String \"", true);
                                    logWrite(vars.stringTag);
                                    logWrite("\" is interpretable as neither decimal nor hex", true);
                                    vars.error = true;
                                    logWrite("Debug: Exiting early due to error", true);
                                    goto end;
                                }
                            }

                            if (vars.elementInfo->type == ElementSet::CUSTOM) {
                                logWrite("Debug: Setting INT parameter for custom element", true);
                                vars.elementXMLFile->setParameter(vars.customParamName, vars.intTemp);
                            } else {
                                logWrite("Debug: Storing INT in buffer", true);
                                vars.intBuffer[vars.paramPosition] = vars.intTemp;
                            }
                            break;
                        }

                        case TYPE_FLOAT: {
                            logWrite("Debug: Casting string to FLOAT", true);

                            // Try decimal conversion
                            if (!stringToFloat(vars.stringTag, &(vars.floatTemp), MAX_TAG_LENGTH)) {
                                logError("wants to cast a string into a float but can't!", xmlFile, i);
                                logWrite(" -> String \"", true);
                                logWrite(vars.stringTag);
                                logWrite("\" is not interpretable as a float type", true);
                                vars.error = true;
                                logWrite("Debug: Exiting early due to error", true);
                                goto end;
                            }

                            if (vars.elementInfo->type == ElementSet::CUSTOM) {
                                logWrite("Debug: Setting FLOAT parameter for custom element", true);
                                vars.elementXMLFile->setParameter(vars.customParamName, vars.floatTemp);
                            } else {
                                logWrite("Debug: Storing FLOAT in buffer", true);
                                vars.floatBuffer[vars.paramPosition] = vars.floatTemp;
                            }
                            break;
                        }

                        case TYPE_STRING: {
                            logWrite("Debug: Handling STRING parameter", true);

                            if (vars.elementInfo->type == ElementSet::CUSTOM) {
                                logWrite("Debug: Setting STRING parameter for custom element", true);
                                vars.elementXMLFile->setParameter(vars.customParamName, vars.stringTag);
                            } else {
                                logWrite("Debug: Copying STRING to buffer", true);
                                memcpy(vars.stringBuffer[vars.paramPosition], vars.stringTag, vars.stringTagLength);

                                // Null-terminate if space allows
                                if (vars.stringTagLength < MAX_TAG_LENGTH) {
                                    vars.stringBuffer[vars.paramPosition][vars.stringTagLength + 1] = '\0';
                                }
                            }
                            break;
                        }

                        default: {
                            logError("failed to match the parameter name to a valid type!", xmlFile, i);
                            vars.error = true;
                            logWrite("Debug: Exiting early due to error", true);
                            goto end;
                        }
                    }

                    vars.paramState = LABEL;

                    break;
                }

                default:
                    logWrite("Debug: Unexpected PARAMS state encountered", true);
                    break;
            }
        }

        logWrite("Done iteration!\n\n");

    }


    /*   Clean up   */

    end:

    // Free buffers
    delete[] vars.intBuffer;
    delete[] vars.floatBuffer;

    for (int i = 0; i < PARAMS_BUFFER_SIZE; i++)
        delete[] vars.stringBuffer[i];

    delete[] vars.stringBuffer;

    // Delete any items in the stack
    for (vars.stack->iterStart(0); vars.stack->iterHasNext(); vars.stack->iterNext())
        delete vars.stack->iterGetObj();

    delete vars.stack;

    // Return nullptr if there was an error
    if (vars.error) return nullptr;

    // Return the last popped element
    // If everything went right, this should have everything else in the file as children elements
    return vars.lastPopped;

}

Window* XML::buildWindow(const char* fileName) {
    return nullptr;
}
