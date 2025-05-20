
#include "application.h"
#include "util/crash_handler.h"

int main(int argc, char* argv[]) {

    GLT::attach_crash_handler();
    GLT::application app = GLT::application(argc, argv);
    app.run();
    
    return EXIT_SUCCESS;
}
