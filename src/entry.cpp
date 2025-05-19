
#include <iostream>

#include "application.h"

#include <util/pch.h>
#include "util/crash_handler.h"
#include "util/io/logger.h"
#include "platform/window.h"

#include "events/event.h"


void on_event(event& event) {

    // PROFILE_FUNCTION();

    // // application events
    // event_dispatcher dispatcher(event);
    // dispatcher.dispatch<window_close_event>(BIND_FUNKTION(application::on_window_close));
    // dispatcher.dispatch<window_resize_event>(BIND_FUNKTION(application::on_window_resize));
    // dispatcher.dispatch<window_refresh_event>(BIND_FUNKTION(application::on_window_refresh));
    // dispatcher.dispatch<window_focus_event>(BIND_FUNKTION(application::on_window_focus));

    // // none application events
    // if (!event.handled) {
    //     for (auto layer = m_layerstack->end(); layer != m_layerstack->begin(); ) {

    //         (*--layer)->on_event(event);
    //         if (event.handled)
    //             break;
    //     }
    // }
}

int main(int argc, char* argv[]) {

    std::cout << "Hello" << std::endl;

    attach_crash_handler();
    application app = application(argc, argv);
    app.run();
    
    return EXIT_SUCCESS;
}
