# Controllers module

HEADERS += $$SK_CORE/controllers/WController.h \
           $$SK_CORE/controllers/WController_p.h \
           $$SK_CORE/controllers/WControllerApplication.h \
           $$SK_CORE/controllers/WControllerApplication_p.h \
           $$SK_CORE/controllers/WControllerFile.h \
           $$SK_CORE/controllers/WControllerFile_p.h \
           $$SK_CORE/controllers/WControllerXml.h \
           $$SK_CORE/controllers/WControllerNetwork.h \
           $$SK_CORE/controllers/WControllerNetwork_p.h \
           $$SK_CORE/controllers/WControllerDownload.h \
           $$SK_CORE/controllers/WControllerDownload_p.h \
           $$SK_GUI/controllers/WControllerPlaylist.h \
           $$SK_GUI/controllers/WControllerPlaylist_p.h \
           $$SK_MEDIA/controllers/WControllerMedia.h \
           $$SK_MEDIA/controllers/WControllerMedia_p.h \
           src/controllers/ControllerCore.h \

SOURCES += $$SK_CORE/controllers/WController.cpp \
           $$SK_CORE/controllers/WControllerApplication.cpp \
           $$SK_CORE/controllers/WControllerFile.cpp \
           $$SK_CORE/controllers/WControllerXml.cpp \
           $$SK_CORE/controllers/WControllerNetwork.cpp \
           $$SK_CORE/controllers/WControllerDownload.cpp \
           $$SK_GUI/controllers/WControllerPlaylist.cpp \
           $$SK_GUI/controllers/WControllerPlaylist_patch.cpp \
           $$SK_MEDIA/controllers/WControllerMedia.cpp \
           src/controllers/ControllerCore.cpp \

