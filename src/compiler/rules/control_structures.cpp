#include "control_structures.hpp"
#include <sstream>

using namespace carma;

rules::control_structures::control_structures()
{
}


rules::control_structures::~control_structures()
{
}


CarmaScopes rules::control_structures::handleIfStatement(CarmaScope& aScope, token_list &tokens_, token_entry& start_entry_, token_entry& end_entry_)
{
    CarmaScopes scopes = CarmaScopes();
    std::stringstream stream;
    
    auto current_token = start_entry_;
    auto statementStart = current_token;
    auto if_start = start_entry_;
    int block_counter = 0;
    bool hasEncounteredCondition = false;

    // Validate that there is no wierd shit in between the if and the ().
    // example: if blaa () < should throw syntax error
    // if () < should throw syntax error, empty condition
    // if (true) < valid
    if (std::next(current_token)->val != "(") {
        throw CarmaSyntaxErrorException("Syntax error");
    }
    // Move past the condition
    for (current_token; current_token != end_entry_ && (!hasEncounteredCondition || block_counter > 0); ++current_token) {
        if (current_token->type == carma::type::EMPTY)
            continue;

        if (current_token->val == "(") {
            if (!hasEncounteredCondition) {
                statementStart = current_token;
            }
            hasEncounteredCondition = true;
            block_counter++;
        }
        if (current_token->val == ")")
            block_counter--;
    }
    // Compile between start_entry_ and current_token for the condition call
    auto statementScope = CarmaScope(&aScope, compiler::context::Type::STATEMENT, tokens_, statementStart, current_token).Compile();

    if (statementScope == "")  // condition must at least have one character
        throw CarmaSyntaxErrorException("Empty condition!");

    stream << "if " << statementScope << " then {";
    
    int onEnd = 0;
    while (current_token->val == "{" || current_token->val == "else") {

        current_token++; // skip the {
        bool isInContent = false;
        if (current_token->val == "{") {
            stream << " else {";
            current_token++; // skip the else
            isInContent = true;
        }

        // figure out the else if construction
        if (current_token->val == "if" && !isInContent) {
            stream << " else {";
            current_token++; // skip the if
            onEnd++;

            // process the if statement
            block_counter = 0;
            statementStart = current_token;
            hasEncounteredCondition = false;

            if (current_token->val != "(") {
                throw CarmaSyntaxErrorException("Syntax error");
            }
            // Move past the condition
            for (current_token; current_token != end_entry_ && (!hasEncounteredCondition || block_counter > 0); ++current_token) {
                if (current_token->type == carma::type::EMPTY)
                    continue;

                if (current_token->val == "(") {
                    if (!hasEncounteredCondition) {
                        statementStart = current_token;
                    }
                    hasEncounteredCondition = true;
                    block_counter++;
                }
                if (current_token->val == ")")
                    block_counter--;
            }
            // Compile between current_token and current_token for the condition call
            CarmaScope statementScope = CarmaScope(&aScope, compiler::context::Type::SCOPE, tokens_, statementStart, current_token);
            stream << "if " << statementScope.Compile() << " then {";

            current_token++; // advance past the )
            
        }
        if (std::prev(current_token)->val == "{") {
            current_token--;
        }
        bool hasEncounteredCodeBlock = false;
        auto block_start_token = current_token;
        block_counter = 0;
        // Move until the last bit of the if statement
        for (current_token; current_token != end_entry_ && (!hasEncounteredCodeBlock || block_counter > 0); ++current_token) {
            if (current_token->type == carma::type::EMPTY)
                continue;

            if (current_token->val == "{") {
                if (!hasEncounteredCodeBlock) { // Store our block token for use later
                    block_start_token = current_token;
                }
                hasEncounteredCodeBlock = true;
                block_counter++;
            }
            if (current_token->val == "}")
                block_counter--;
        }
        current_token--;

        if (block_counter > 0)
            throw CarmaSyntaxErrorException("syntax error!"); //syntax error
        
        if (current_token != block_start_token) {
            block_start_token++;

            // code block is: block_start_token till current_token
            // process contents within code block regulary
            CarmaScope contentScope = CarmaScope(&aScope, compiler::context::Type::SCOPE, tokens_, block_start_token, current_token);
            auto block_content = contentScope.Compile();
            stream << "" << block_content << "}";
            scopes.push_back(contentScope);
        }
        else {
            stream << "}";
        }
        
        if (current_token != end_entry_) {
            if (std::next(current_token)->val == "else") {
                current_token++;
            }
        }
    }
    for (int amount = 0; amount < onEnd; amount++) {
        stream << "}";
    }

    if (current_token != end_entry_) {
        if (std::next(current_token)->val == ";")
            current_token++;
    }
    stream << ";" << std::endl; // final semi-column

    // clear out everything from start until current token
    compiler::empty_tokens(tokens_, if_start, current_token);

    // replace current start value by stream.str(). Set proper type
    // set our code block contents
    current_token->type = carma::type::CODEBLOCK;
    current_token->val = stream.str();
    start_entry_ == current_token;

    return scopes;
}

CarmaScopes rules::control_structures::handleSwitchStatement(CarmaScope& aScope, token_list &tokens_, token_entry& start_entry_, token_entry& end_entry_)
{
    CarmaScopes scopes = CarmaScopes();
    std::stringstream stream;
    auto current_token = start_entry_;
    auto statementStart = current_token;
    int block_counter = 0;
    bool hasEncounteredCondition = false;

    if (current_token->val != "switch")
        throw CarmaSyntaxErrorException("Not a switch statement");

    // Validate that there is no wierd shit in between the switch and the ().
    // example: if blaa () < should throw syntax error
    // if () < should throw syntax error, empty condition
    // if (true) < valid
    if (std::next(current_token)->val != "(") {
        throw CarmaSyntaxErrorException("Syntax error");
    }
    // Move past the condition
    for (current_token; current_token != end_entry_ && (!hasEncounteredCondition || block_counter > 0); ++current_token) {
        if (current_token->type == carma::type::EMPTY)
            continue;

        if (current_token->val == "(") {
            if (!hasEncounteredCondition) {
                statementStart = current_token;
            }
            hasEncounteredCondition = true;
            block_counter++;
        }
        if (current_token->val == ")")
            block_counter--;
    }
    // Compile between start_entry_ and current_token for the condition call
    auto statementScope = CarmaScope(&aScope, compiler::context::Type::STATEMENT, tokens_, statementStart, current_token).Compile();
    if (statementScope == "")  // condition must at least have one character
        throw CarmaSyntaxErrorException("Empty condition!");
    stream << "switch " << statementScope << " do {" << std::endl;

    if (current_token == end_entry_)
        throw CarmaSyntaxErrorException("On the end already");

    // Handle the case statements
    while (std::next(current_token)->val == "case" || std::next(current_token)->val == "default") {
        current_token++;

        // Move past the case condition
        std::string case_statement = "";
        if (current_token->val != "default") {
            hasEncounteredCondition = false;
            block_counter = 0;
            for (current_token; current_token != end_entry_ && (!hasEncounteredCondition || block_counter > 0); ++current_token) {
                if (current_token->type == carma::type::EMPTY)
                    continue;

                if (current_token->val == "(") {
                    if (!hasEncounteredCondition) {
                        statementStart = current_token;
                    }
                    hasEncounteredCondition = true;
                    block_counter++;
                }
                if (current_token->val == ")")
                    block_counter--;
            }
            case_statement = CarmaScope(&aScope, compiler::context::Type::STATEMENT, tokens_, statementStart, current_token).Compile();

            if (case_statement == "")
                throw CarmaSyntaxErrorException("Empty condition statement in case label");
        }
        else {
            current_token++;
            if (current_token->val == ":")
                current_token++;

            // count defaults and add constraint only one default allowed in a switch
        }

        if (current_token->val == ":")
            current_token++;

        auto start_case = current_token; // store the start of our case block

        block_counter = 0;
        bool labelSeen = false;

        // TODO find the end of our case / default. This code may need to be improved
        for (current_token; current_token != end_entry_ && (!labelSeen || block_counter > 0); ++current_token) {
            if (current_token->type == carma::type::EMPTY)
                continue;
            if (current_token->val == "case" || current_token->val == "default") {
                block_counter++;
            }
            if (current_token->val == "break") {
                block_counter--;
                labelSeen = true;
            }
        }
        if (block_counter > 0) 
            throw CarmaSyntaxErrorException("Missing break keyword behind case label");

        current_token--;

        if (current_token->val != "break") {
            // syntax error        
        }

        auto case_scope_compiled = CarmaScope(&aScope, compiler::context::Type::SCOPE, tokens_, start_case, current_token).Compile();
        if (case_statement != "") {
            stream << "case " << case_statement << ": {" << case_scope_compiled << "};";
        }
        else {
            stream << "default: {" << case_scope_compiled << "};";
        }

        current_token++; // skip the break label
        while (current_token->val == ";" || current_token->val == "break")
            current_token++; // skip the ; behind the break

        if (current_token == end_entry_)
            throw CarmaSyntaxErrorException("On the end already");
        
        //if (current_token == end_entry_ || std::next(current_token) == end_entry_);
       //     throw std::exception("On the end already");
        if (current_token->val == "case" || current_token->val == "default")
            current_token--;

    }
    while (current_token->val == ";" || current_token->val == "break")
        current_token++; // skip the ; behind the break

    if (current_token->val != "}") {
        throw CarmaMissingBracketException("Missing closing bracket on switch statement");  
    }

    if (std::next(current_token) != end_entry_) {
        if (std::next(current_token)->val != ";") {
            std::next(current_token)->type == carma::type::EMPTY;
        }
    }
    stream << "};";
    
    compiler::empty_tokens(tokens_, start_entry_, current_token);
    current_token->type == carma::type::CODEBLOCK;
    current_token->val = stream.str();

    start_entry_ = current_token--;

    return scopes;
}

CarmaScopes rules::control_structures::handleWhileStatement(CarmaScope& aScope, token_list &tokens_, token_entry& start_entry_, token_entry& end_entry_)
{
    CarmaScopes scopes = CarmaScopes();
    std::stringstream stream;

    auto current_token = start_entry_;
    auto statementStart = current_token;
    int block_counter = 0;
    bool hasEncounteredCondition = false;

    // Validate that there is no wierd shit in between the if and the ().
    // example: if blaa () < should throw syntax error
    // if () < should throw syntax error, empty condition
    // if (true) < valid
    if (std::next(current_token)->val != "(") {
        throw CarmaSyntaxErrorException("Syntax error");
    }
    // Move past the condition
    for (current_token; current_token != end_entry_ && (!hasEncounteredCondition || block_counter > 0); ++current_token) {
        if (current_token->type == carma::type::EMPTY)
            continue;

        if (current_token->val == "(") {
            if (!hasEncounteredCondition) {
                statementStart = current_token;
            }
            hasEncounteredCondition = true;
            block_counter++;
        }
        if (current_token->val == ")")
            block_counter--;
    }
    // Compile between start_entry_ and current_token for the condition call
    auto statementScope = CarmaScope(&aScope, compiler::context::Type::STATEMENT, tokens_, statementStart, current_token).Compile();
    if (statementScope == "")  // condition must at least have one character
        throw CarmaSyntaxErrorException("Empty condition!");

    stream << "while {(" << statementScope << ")} do {" << std::endl;
    if (current_token->val != "{")
        throw CarmaMissingBracketException("Missing {");
    
    bool hasEncounteredCodeBlock = false;
    auto block_start_token = current_token;
    block_counter = 0;
    // Move until the last bit of the if statement
    for (current_token; current_token != end_entry_ && (!hasEncounteredCodeBlock || block_counter > 0); ++current_token) {
        if (current_token->type == carma::type::EMPTY)
            continue;

        if (current_token->val == "{") {
            if (!hasEncounteredCodeBlock) { // Store our block token for use later
                block_start_token = current_token;
            }
            hasEncounteredCodeBlock = true;
            block_counter++;
        }
        if (current_token->val == "}")
            block_counter--;
    }
    if (current_token == end_entry_ && block_counter > 0)
        throw CarmaSyntaxErrorException("syntax error!"); //syntax error
    block_start_token++;

    // code block is: block_start_token till current_token
    // process contents within code block regulary
    CarmaScope contentScope = CarmaScope(&aScope, compiler::context::Type::SCOPE, tokens_, block_start_token, current_token);
    stream << "" << contentScope.Compile() << ";" << std::endl;

    scopes.push_back(contentScope);
    
    // clear out everything from start until current token
    compiler::empty_tokens(tokens_, start_entry_, current_token);

    // replace current start value by stream.str(). Set proper type
    // set our code block contents
    current_token->type = carma::type::CODEBLOCK;
    current_token->val = stream.str();

    return scopes;
}

CarmaScopes rules::control_structures::handleWaitUntilStatement(CarmaScope& aScope, token_list &tokens_, token_entry& start_entry_, token_entry& end_entry_)
{
    CarmaScopes scopes = CarmaScopes();

    return scopes;
}
