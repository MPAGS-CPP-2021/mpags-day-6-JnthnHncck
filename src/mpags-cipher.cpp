#include "CipherFactory.hpp"
#include "CipherMode.hpp"
#include "CipherType.hpp"
#include "ProcessCommandLine.hpp"
#include "TransformChar.hpp"
#include "CustomExceptions.hpp"

#include <cctype>
#include <fstream>
#include <iostream>
#include <string>
#include <vector>
#include <future>
#include <thread>

int main(int argc, char* argv[])
{
    // Convert the command-line arguments into a more easily usable form
    const std::vector<std::string> cmdLineArgs{argv, argv + argc};

    // Options that might be set by the command-line arguments
    ProgramSettings settings{
        false, false, "", "", "", CipherMode::Encrypt, CipherType::Caesar};

    // Process command line arguments
    try {
        processCommandLine(cmdLineArgs, settings);
    } catch (const MissingArgument& e) {
        std::cerr << "[error] Missing argument: " << e.what() << std::endl;
        return 1;
    }

    // Handle help, if requested
    if (settings.helpRequested) {
        // Line splitting for readability
        std::cout
            << "Usage: mpags-cipher [-h/--help] [--version] [-i <file>] [-o <file>] [-c <cipher>] [-k <key>] [--encrypt/--decrypt]\n\n"
            << "Encrypts/Decrypts input alphanumeric text using classical ciphers\n\n"
            << "Available options:\n\n"
            << "  -h|--help        Print this help message and exit\n\n"
            << "  --version        Print version information\n\n"
            << "  -i FILE          Read text to be processed from FILE\n"
            << "                   Stdin will be used if not supplied\n\n"
            << "  -o FILE          Write processed text to FILE\n"
            << "                   Stdout will be used if not supplied\n\n"
            << "                   Stdout will be used if not supplied\n\n"
            << "  -c CIPHER        Specify the cipher to be used to perform the encryption/decryption\n"
            << "                   CIPHER can be caesar, playfair, or vigenere - caesar is the default\n\n"
            << "  -k KEY           Specify the cipher KEY\n"
            << "                   A null key, i.e. no encryption, is used if not supplied\n\n"
            << "  --encrypt        Will use the cipher to encrypt the input text (default behaviour)\n\n"
            << "  --decrypt        Will use the cipher to decrypt the input text\n\n"
            << std::endl;
        // Help requires no further action, so return from main
        // with 0 used to indicate success
        return 0;
    }

    // Handle version, if requested
    // Like help, requires no further action,
    // so return from main with zero to indicate success
    if (settings.versionRequested) {
        std::cout << "0.5.0" << std::endl;
        return 0;
    }

    // Initialise variables
    char inputChar{'x'};
    std::string inputText;

    // Read in user input from stdin/file
    if (!settings.inputFile.empty()) {
        // Open the file and check that we can read from it
        std::ifstream inputStream{settings.inputFile};
        if (!inputStream.good()) {
            std::cerr << "[error] failed to create istream on file '"
                      << settings.inputFile << "'" << std::endl;
            return 1;
        }

        // Loop over each character from the file
        while (inputStream >> inputChar) {
            inputText += transformChar(inputChar);
        }

    } else {
        // Loop over each character from user input
        // (until Return then CTRL-D (EOF) pressed)
        while (std::cin >> inputChar) {
            inputText += transformChar(inputChar);
        }
    }

    // Request construction of the appropriate cipher
    try {
        auto testcipher = cipherFactory(settings.cipherType, settings.cipherKey);
    } catch (const InvalidKey& e) {
        std::cerr << "[error] Invalid Key: " << e.what() << std::endl;
        return 1;
    }
    
    auto cipher = cipherFactory(settings.cipherType, settings.cipherKey);

    //we want four threads to start off with
    size_t nThreads{4};
    std::vector< std::future< std::string > > futures{};

    size_t inLen{inputText.length()};
    size_t div{inLen / nThreads};
    // The final chunk may be larger, as it takes the extra characters when the division is not exact
    size_t last{div + inLen % nThreads};

    for (size_t i{0}; i < nThreads; i++) {
        size_t length{0};
        if (i != (nThreads - 1)) {
            length = div;
        } else {
            length = last;
        }
        // Makes a substring, at a postion depending on i
        std::string chunk{inputText.substr(div * i, length)};
        // We add a new thread to our thread vector, and then tell it to apply the cipher to its chunk
        futures.push_back( std::async(std::launch::async, [&cipher, chunk, &settings] () { return(cipher->applyCipher(chunk, settings.cipherMode)); } ));
    }

    // We iterate over each future in the vector, and only move on to the next when they are finished
    for (auto& f : futures) {
        std::future_status status{std::future_status::ready};
        do {
            //We check if the future is done every second
            status = f.wait_for(std::chrono::seconds(1));
            if (status == std::future_status::timeout) {
                std::cout << "Thread waiting..." << std::endl;
            } else if (status == std::future_status::ready) {
                std::cout << "Thread complete!" << std::endl;
            }
        } while (status != std::future_status::ready);
    }

    // Now that all futures are finished, we can safely use all of their outputs
    std::string outputText{""};
    for (auto&f : futures) {
        // Recombines all the encrypted chunks
        outputText = outputText + f.get();
    }

    // Output the encrypted/decrypted text to stdout/file
    if (!settings.outputFile.empty()) {
        // Open the file and check that we can write to it
        std::ofstream outputStream{settings.outputFile};
        if (!outputStream.good()) {
            std::cerr << "[error] failed to create ostream on file '"
                      << settings.outputFile << "'" << std::endl;
            return 1;
        }

        // Print the encrypted/decrypted text to the file
        outputStream << outputText << std::endl;

    } else {
        // Print the encrypted/decrypted text to the screen
        std::cout << outputText << std::endl;
    }

    // No requirement to return from main, but we do so for clarity
    // and for consistency with other functions
    return 0;
}
