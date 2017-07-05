# CMake version required
cmake_minimum_required( VERSION 2.8 )

# Declare a Cortex Project
project( Gaffer )

# Declare the version
set( GAFFER_MILESTONE_VERSION 0 )
set( GAFFER_MAJOR_VERSION 34 )
set( GAFFER_MINOR_VERSION 0 )
set( GAFFER_PATCH_VERSION 0 )

add_definitions( -DGAFFER_MILESTONE_VERSION=${GAFFER_MILESTONE_VERSION}
                 -DGAFFER_MAJOR_VERSION=${GAFFER_MAJOR_VERSION}
                 -DGAFFER_MINOR_VERSION=${GAFFER_MINOR_VERSION}
                 -DGAFFER_PATCH_VERSION=${GAFFER_PATCH_VERSION} )

option( WITH_TESTS "Compile tests" OFF )

#-******************************************************************************
#-******************************************************************************
# PLATFORM SPECIFIC
#-******************************************************************************
#-******************************************************************************

# Windows flag
set( WINDOWS FALSE )
if( "${CMAKE_SYSTEM_NAME}" MATCHES "Windows" )
  set( WINDOWS TRUE )
endif()

set( DARWIN FALSE )
if( "${CMAKE_SYSTEM_NAME}" MATCHES "Darwin" )
  set( DARWIN TRUE )
  if( ${CMAKE_SYSTEM_VERSION} MATCHES "13." )
       ADD_DEFINITIONS( "-std=c++11" )
  endif()
endif()

set( LINUX FALSE )
if( "${CMAKE_SYSTEM_NAME}" MATCHES "Linux" )
  set( LINUX TRUE )
endif()

# Set some debug vs opt flags
IF ( "${CMAKE_BUILD_TYPE}" MATCHES "Debug" )
  ADD_DEFINITIONS( -DDEBUG=1 -UNDEBUG )
  IF ( NOT WINDOWS )
    ADD_DEFINITIONS( -pipe -Wall -O0)
  ENDIF()
ELSEIF ( "${CMAKE_BUILD_TYPE}" MATCHES "Release" )
  ADD_DEFINITIONS( -DNDEBUG=1 -UDEBUG )
  IF ( NOT WINDOWS )
    ADD_DEFINITIONS( -pipe -Wall -Werror -O3 -DNDEBUG -DBOOST_DISABLE_ASSERTS -Wno-unused-local-typedefs -Wno-strict-aliasing -Wno-maybe-uninitialized ) #-Wno-return-type )
  ENDIF()
ENDIF()

if( ${WINDOWS} )
  add_definitions( -DGAFFER_PLATFORM=win)
  add_definitions( -DOPENEXR_DLL -DNOMINMAX -D__PRETTY_FUNCTION__=__FUNCSIG__ -DBOOST_ALL_DYN_LINK )
  set( CMAKE_CXX_FLAGS ${CMAKE_CXX_FLAGS} /MP )
  set( PLATFORM_LIBS winmm.lib )
endif()

#-******************************************************************************
#-******************************************************************************
# GLOBAL PACKAGE & INCLUDE/LINK DEPENDENCY
#-******************************************************************************
#-******************************************************************************

# check in the source directory
set( CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${PROJECT_SOURCE_DIR}/cmake )

find_package( TBB REQUIRED )
find_package( PythonLibs 2.7 REQUIRED )
find_package( Boost REQUIRED COMPONENTS signals iostreams filesystem date_time wave regex system chrono python )
find_package( IlmBase REQUIRED )
find_package( OpenEXR REQUIRED )
find_package( Cortex REQUIRED )
find_package( OpenColorIO REQUIRED )
find_package( OpenImageIO REQUIRED )
find_package( OSL REQUIRED )
find_package( FreeType REQUIRED )
find_package( PNG REQUIRED ) # For freetype linking
find_package( Arnold REQUIRED )
find_package( OpenGL REQUIRED )
find_package( GLEW REQUIRED )
find_package( Qt5 5.6 REQUIRED Core Gui OpenGL )
find_package( Appleseed REQUIRED )

include_directories( ${TBB_INCLUDE_DIRS} )
include_directories( ${PYTHON_INCLUDE_DIRS} )
include_directories( ${Boost_INCLUDE_DIRS} )
include_directories( ${ILMBASE_INCLUDE_DIR} )
include_directories( ${OPENEXR_INCLUDE_DIRS} )
include_directories( ${Cortex_INCLUDE_DIRS} )
include_directories( ${FREETYPE_INCLUDE_DIRS} )
include_directories( ${CORTEX_ARNOLD_INCLUDE_PATH} )
include_directories( ${APPLESEED_INCLUDE_DIRS} )

link_directories( ${TBB_LIBRARY_DIRS} )
link_directories( ${Boost_LIBRARY_DIRS} )

# Boost definitions
add_definitions( -DBOOST_FILESYSTEM_VERSION=3 )
add_definitions( -DBOOST_FILESYSTEM_NO_DEPRICATED )
add_definitions( -DBOOST_SIGNALS_NO_DEPRECATION_WARNING )
add_definitions( -DBOOST_PYTHON_MAX_ARITY=20 )

# Base libs to link to
set( BASE_LIBS 
     ${Boost_SIGNALS_LIBRARY}
     ${Boost_IOSTREAMS_LIBRARY}
     ${Boost_FILESYSTEM_LIBRARY}
     ${Boost_DATE_TIME_LIBRARY}
     ${Boost_THREAD_LIBRARY}
     ${Boost_WAVE_LIBRARY}
     ${Boost_REGEX_LIBRARY}
     ${Boost_SYSTEM_LIBRARY}
     ${Boost_CHRONO_LIBRARY}
     ${TBB_LIBRARIES}
     ${Ilmbase_HALF_LIBRARY}
     ${Ilmbase_IEX_LIBRARY}
     ${Ilmbase_IEXMATH_LIBRARY}
     ${Ilmbase_ILMTHREAD_LIBRARY}
     ${Ilmbase_IMATH_LIBRARY}
     ${Openexr_ILMIMF_LIBRARY}
     ${Cortex_IECORE_LIBRARY})

set( BASE_PYTHON_LIBS 
     ${PYTHON_LIBRARIES}
     ${Boost_PYTHON_LIBRARY}
     ${Cortex_IECOREPYTHON_LIBRARY}
     Gaffer)

#-******************************************************************************
#-******************************************************************************
# GAFFER
#-******************************************************************************
#-******************************************************************************

# Includes
include_directories( ${CMAKE_SOURCE_DIR}/include )

set( GAFFER_LIB_NAMES 
  Gaffer
  GafferAppleseed
#  GafferArnold
#  GafferCortex
  GafferDispatch
  GafferImage
  GafferImageUI
  GafferOSL
#  GafferRenderMan
  GafferScene
  GafferSceneUI
  GafferUI
  )

set( GAFFERBINDINGS_LIB_NAMES 
  Gaffer
#  GafferCortex
  GafferDispatch
  GafferImage
  GafferImageUI
  GafferScene
  GafferSceneUI
  GafferUI
  )

set( GAFFERTEST_LIB_NAMES 
  Gaffer
  GafferImage
  GafferScene
  )

set( GAFFER_PYTHON_NAMES 
  Gaffer
  GafferAppleseed
#  GafferArnold
#  GafferCortex
  GafferDispatch
  GafferImage
  GafferOSL
#  GafferRenderMan
  GafferScene
  GafferTractor
  )

# Get lists of files
foreach( GAFFER_LIB ${GAFFER_LIB_NAMES} )
  string( TOUPPER ${GAFFER_LIB} _GAFFER_LIB )
  file( GLOB_RECURSE ${_GAFFER_LIB}_CXX_FILES RELATIVE ${CMAKE_SOURCE_DIR} src/${GAFFER_LIB}/*.cpp )
  file( GLOB_RECURSE ${_GAFFER_LIB}_H_FILES RELATIVE ${CMAKE_SOURCE_DIR} include/${GAFFER_LIB}/*.h include/${GAFFER_LIB}/*.inl )
  set( ${_GAFFER_LIB}_CXX_MODULE_FILES src/${GAFFER_LIB}Module/${GAFFER_LIB}Module.cpp)
endforeach()

# Get test lists of files
foreach( GAFFER_LIB ${GAFFERTEST_LIB_NAMES} )
  string( TOUPPER ${GAFFER_LIB} _GAFFER_LIB )
  file( GLOB_RECURSE ${_GAFFER_LIB}TEST_CXX_FILES RELATIVE ${CMAKE_SOURCE_DIR} src/${GAFFER_LIB}Test/*.cpp )
  file( GLOB_RECURSE ${_GAFFER_LIB}TEST_H_FILES RELATIVE ${CMAKE_SOURCE_DIR} include/${GAFFER_LIB}Test/*.h include/${GAFFER_LIB}Test/*.inl )
  set( ${_GAFFER_LIB}TEST_CXX_MODULE_FILES src/${GAFFER_LIB}TestModule/${GAFFER_LIB}TestModule.cpp)
endforeach()

# Get binding list of files
foreach( GAFFER_LIB ${GAFFERBINDINGS_LIB_NAMES} )
  string( TOUPPER ${GAFFER_LIB} _GAFFER_LIB )
  file( GLOB_RECURSE ${_GAFFER_LIB}BINDINGS_CXX_FILES RELATIVE ${CMAKE_SOURCE_DIR} src/${GAFFER_LIB}Bindings/*.cpp )
  file( GLOB_RECURSE ${_GAFFER_LIB}BINDINGS_H_FILES RELATIVE ${CMAKE_SOURCE_DIR} include/${GAFFER_LIB}Bindings/*.h include/${GAFFER_LIB}Bindings/*.inl )
endforeach()

# Get python list of files
foreach( GAFFER_LIB ${GAFFER_PYTHON_NAMES} )
  string( TOUPPER ${GAFFER_LIB} _GAFFER_LIB )
  file( GLOB_RECURSE ${_GAFFER_LIB}_PY_FILES RELATIVE ${CMAKE_SOURCE_DIR} python/${GAFFER_LIB}/*.py 
    python/${GAFFER_LIB}Test/*.py python/${GAFFER_LIB}UI/*.py python/${GAFFER_LIB}UITest/*.py )
endforeach()

# Gaffer
add_library( Gaffer SHARED ${GAFFER_CXX_FILES} )
target_link_libraries( Gaffer ${BASE_LIBS} )

# Gaffer Bindings
add_library( GafferBindings SHARED ${GAFFERBINDINGS_CXX_FILES} )
target_link_libraries( GafferBindings ${BASE_LIBS} ${BASE_PYTHON_LIBS} )

# Gaffer Module
add_library( _Gaffer MODULE ${GAFFER_CXX_MODULE_FILES} )
target_link_libraries( _Gaffer ${BASE_LIBS} ${BASE_PYTHON_LIBS} GafferBindings )

# Gaffer Test
add_library( GafferTest SHARED ${GAFFERTEST_CXX_FILES} )
target_link_libraries( GafferTest ${BASE_LIBS} Gaffer )

# Gaffer Test Module
add_library( _GafferTest MODULE ${GAFFERTEST_CXX_MODULE_FILES} )
target_link_libraries( _GafferTest ${BASE_LIBS} ${BASE_PYTHON_LIBS} GafferBindings GafferTest )

# GafferUI
add_library( GafferUI SHARED ${GAFFERUI_CXX_FILES} )
target_link_libraries( GafferUI ${BASE_LIBS} Gaffer ${Iex_LIB} ${Cortex_IECOREGL_LIBRARY} ${OPENGL_LIBRARIES} ${GLEW_LIBRARY} ${PLATFORM_LIBS} )

# GafferUI Bindings
add_library( GafferUIBindings SHARED ${GAFFERUIBINDINGS_CXX_FILES} )
target_link_libraries( GafferUIBindings ${BASE_LIBS} ${BASE_PYTHON_LIBS} GafferUI GafferBindings Qt5::Core Qt5::Gui Qt5::OpenGL )
target_compile_definitions( GafferUIBindings PRIVATE -DQT_NO_KEYWORDS )

# GafferUI Python Module
add_library( _GafferUI MODULE ${GAFFERUI_CXX_MODULE_FILES} )
target_link_libraries( _GafferUI ${BASE_LIBS} ${BASE_PYTHON_LIBS} GafferBindings GafferUIBindings )

# GafferDisptach
add_library( GafferDispatch SHARED ${GAFFERDISPATCH_CXX_FILES} )
target_link_libraries( GafferDispatch ${BASE_LIBS} Gaffer )

# GafferDispatch Bindings
add_library( GafferDispatchBindings SHARED ${GAFFERDISPATCHBINDINGS_CXX_FILES} )
target_link_libraries( GafferDispatchBindings ${BASE_LIBS} ${BASE_PYTHON_LIBS} GafferBindings GafferDispatch )

# GafferDispatch Python Module
add_library( _GafferDispatch MODULE ${GAFFERDISPATCH_CXX_MODULE_FILES} )
target_link_libraries( _GafferDispatch ${BASE_LIBS} ${BASE_PYTHON_LIBS} GafferBindings GafferDispatchBindings )

# GafferImage
add_library( GafferImage SHARED ${GAFFERIMAGE_CXX_FILES} )
target_link_libraries( GafferImage ${BASE_LIBS} Gaffer GafferDispatch ${ILMBASE_IEX_LIBRARY} ${OPENIMAGEIO_LIBRARIES} ${OPENCOLORIO_LIBRARIES} ${FREETYPE_LIBRARIES} ${PNG_LIBRARIES} )

# GafferImage Bindings
add_library( GafferImageBindings SHARED ${GAFFERIMAGEBINDINGS_CXX_FILES} )
target_link_libraries( GafferImageBindings ${BASE_LIBS} ${BASE_PYTHON_LIBS} GafferBindings GafferImage GafferDispatch )

# GafferImage Python Module
add_library( _GafferImage MODULE ${GAFFERIMAGE_CXX_MODULE_FILES} )
target_link_libraries( _GafferImage ${BASE_LIBS} ${BASE_PYTHON_LIBS} GafferImage GafferBindings GafferImageBindings GafferDispatchBindings )

# GafferImage Test
add_library( GafferImageTest SHARED ${GAFFERIMAGETEST_CXX_FILES} )
target_link_libraries( GafferImageTest ${BASE_LIBS} Gaffer GafferImage ${OPENIMAGEIO_LIBRARIES} )

# GafferImage Test Module
add_library( _GafferImageTest MODULE ${GAFFERIMAGETEST_CXX_MODULE_FILES} )
target_link_libraries( _GafferImageTest ${BASE_LIBS} ${BASE_PYTHON_LIBS} GafferImage GafferImageTest GafferImageBindings )

# GafferImageUI
add_library( GafferImageUI SHARED ${GAFFERIMAGEUI_CXX_FILES} )
target_link_libraries( GafferImageUI ${BASE_LIBS} Gaffer GafferImage GafferUI ${OPENGL_LIBRARIES} ${GLEW_LIBRARY} )

# GafferImageUI Bindings
add_library( GafferImageUIBindings SHARED ${GAFFERIMAGEUIBINDINGS_CXX_FILES} )
target_link_libraries( GafferImageUIBindings ${BASE_LIBS} ${BASE_PYTHON_LIBS} GafferBindings GafferImage GafferDispatch GafferUI GafferImageUI )

# GafferImageUI Python Module
add_library( _GafferImageUI MODULE ${GAFFERIMAGEUI_CXX_MODULE_FILES} )
target_link_libraries( _GafferImageUI ${BASE_LIBS} ${BASE_PYTHON_LIBS} GafferBindings GafferUIBindings GafferImageUIBindings )

# GafferScene
add_library( GafferScene SHARED ${GAFFERSCENE_CXX_FILES} )
target_link_libraries( GafferScene ${BASE_LIBS} Gaffer ${ILMBASE_IEX_LIBRARY} ${Cortex_IECOREGL_LIBRARY} ${Cortex_IECOREALEMBIC_LIBRARY} GafferImage GafferDispatch )

# GafferScene Bindings
add_library( GafferSceneBindings SHARED ${GAFFERSCENEBINDINGS_CXX_FILES} )
target_link_libraries( GafferSceneBindings ${BASE_LIBS} ${BASE_PYTHON_LIBS} GafferScene GafferBindings )

# GafferScene Python Module
add_library( _GafferScene MODULE ${GAFFERSCENE_CXX_MODULE_FILES} )
target_link_libraries( _GafferScene ${BASE_LIBS} ${BASE_PYTHON_LIBS} GafferBindings GafferSceneBindings GafferDispatchBindings )

# GafferScene Test
add_library( GafferSceneTest SHARED ${GAFFERSCENETEST_CXX_FILES} )
target_link_libraries( GafferSceneTest ${BASE_LIBS} Gaffer GafferScene GafferDispatch )

# GafferScene Test Module
add_library( _GafferSceneTest MODULE ${GAFFERSCENETEST_CXX_MODULE_FILES} )
target_link_libraries( _GafferSceneTest ${BASE_LIBS} ${BASE_PYTHON_LIBS} GafferBindings GafferScene GafferSceneTest GafferDispatch )

# GafferSceneUI
add_library( GafferSceneUI SHARED ${GAFFERSCENEUI_CXX_FILES} )
target_link_libraries( GafferSceneUI ${BASE_LIBS} Gaffer GafferUI GafferImage GafferScene GafferImageUI ${ILMBASE_IEX_LIBRARY} ${Cortex_IECOREGL_LIBRARY} ${OPENGL_LIBRARIES} ${GLEW_LIBRARY} )

# GafferSceneUI Bindings
add_library( GafferSceneUIBindings SHARED ${GAFFERSCENEUIBINDINGS_CXX_FILES} )
target_link_libraries( GafferSceneUIBindings ${BASE_LIBS} ${BASE_PYTHON_LIBS} ${Cortex_IECOREGL_LIBRARY} GafferBindings GafferScene GafferUI GafferImageUI GafferSceneUI )

# GafferSceneUI Python Module
add_library( _GafferSceneUI MODULE ${GAFFERSCENEUI_CXX_MODULE_FILES} )
target_link_libraries( _GafferSceneUI ${BASE_LIBS} ${BASE_PYTHON_LIBS} GafferBindings GafferUIBindings GafferSceneUIBindings )

# GafferCortex
#add_library( GafferCortex SHARED ${GAFFERCORTEX_CXX_FILES} )
#target_link_libraries( GafferCortex ${BASE_LIBS} Gaffer GafferDispatch )

# GafferCortex Bindings
#add_library( GafferCortexBindings SHARED ${GAFFERCORTEXBINDINGS_CXX_FILES} )
#target_link_libraries( GafferCortexBindings ${BASE_LIBS} ${BASE_PYTHON_LIBS} GafferBindings GafferCortex GafferDispatch )

# GafferCortex Python Module
#add_library( _GafferCortex MODULE ${GAFFERCORTEX_CXX_MODULE_FILES} )
#target_link_libraries( _GafferCortex ${BASE_LIBS} ${BASE_PYTHON_LIBS} GafferBindings GafferCortexBindings )

# GafferOSL
add_library( GafferOSL SHARED ${GAFFEROSL_CXX_FILES} )
target_link_libraries( GafferOSL ${BASE_LIBS} Gaffer GafferScene GafferImage ${OPENIMAGEIO_LIBRARIES} ${OSL_QUERY_LIBRARY} ${OSL_EXEC_LIBRARY} ${ILMBASE_IEX_LIBRARY} )

# GafferOSL Python Module
add_library( _GafferOSL MODULE ${GAFFERAPPLESEED_CXX_MODULE_FILES} )
target_link_libraries( _GafferOSL ${BASE_LIBS} ${BASE_PYTHON_LIBS} GafferBindings GafferScene GafferImage GafferOSL GafferAppleseed ${ILMBASE_IEX_LIBRARY} )

# GafferAppleseed
file( GLOB_RECURSE IECOREAPPLESEED_PREVIEW_CXX_FILES RELATIVE ${CMAKE_SOURCE_DIR} src/GafferAppleseed/IECoreAppleseedPreview/*.cpp )
add_library( GafferAppleseed SHARED ${GAFFERAPPLESEED_CXX_FILES} ${IECOREAPPLESEED_PREVIEW_CXX_FILES} )
target_link_libraries( GafferAppleseed ${BASE_LIBS} Gaffer GafferDispatch GafferScene ${APPLESEED_LIBRARY} ${Cortex_IECOREAPPLESEED_LIBRARY} ${OPENIMAGEIO_LIBRARIES} ${OSL_QUERY_LIBRARY} )
target_compile_options( GafferAppleseed PRIVATE -DAPPLESEED_ENABLE_IMATH_INTEROP -DAPPLESEED_WITH_OIIO -DAPPLESEED_WITH_OSL -DAPPLESEED_USE_SSE )

# GafferAppleseed Python Module
add_library( _GafferAppleseed MODULE ${GAFFERAPPLESEED_CXX_MODULE_FILES} )
target_link_libraries( _GafferAppleseed ${BASE_LIBS} ${BASE_PYTHON_LIBS} GafferDispatch GafferScene GafferBindings GafferAppleseed )
target_compile_options( _GafferAppleseed PRIVATE -DAPPLESEED_ENABLE_IMATH_INTEROP -DAPPLESEED_WITH_OIIO -DAPPLESEED_WITH_OSL -DAPPLESEED_USE_SSE )

# GafferArnold
#file( GLOB_RECURSE IECOREARNOLD_PREVIEW_CXX_FILES RELATIVE ${CMAKE_SOURCE_DIR} src/GafferArnold/IECoreArnoldPreview/*.cpp )
#add_library( GafferArnold SHARED ${GAFFERARNOLD_CXX_FILES} ${IECOREARNOLD_PREVIEW_CXX_FILES} )
#target_link_libraries( GafferArnold ${BASE_LIBS} Gaffer GafferDispatch GafferScene openvdb ${Cortex_IECOREARNOLD_LIBRARY} )

# GafferArnold Python Module
#add_library( _GafferArnold MODULE ${GAFFERARNOLD_CXX_MODULE_FILES} )
#target_link_libraries( _GafferArnold ${BASE_LIBS} ${BASE_PYTHON_LIBS} GafferScene GafferBindings GafferDispatch GafferArnold GafferBindings )

# GafferRenderMan
#add_library( GafferRenderMan SHARED ${GAFFERRENDERMAN_CXX_FILES} )
#target_link_libraries( GafferRenderMan ${BASE_LIBS} Gaffer GafferDispatch GafferScene ${Cortex_IECORERI_LIBRARY} )

# GafferRenderMan Python Module
#add_library( _GafferRenderMan MODULE ${GAFFERRENDERMAN_CXX_MODULE_FILES} )
#target_link_libraries( _GafferRenderMan ${BASE_LIBS} ${BASE_PYTHON_LIBS} GafferScene GafferBindings GafferDispatch GafferRenderMan GafferBindings )

# Python modules need special naming
foreach( GAFFER_LIB ${GAFFER_LIB_NAMES} )
  if( DARWIN )
    set_target_properties( _${GAFFER_LIB} PROPERTIES SUFFIX ".so" )
    set_target_properties( _${GAFFER_LIB} PROPERTIES PREFIX "" )
  elif( WINDOWS )
    set_target_properties( _${GAFFER_LIB} PROPERTIES SUFFIX ".pyd" )
  else()
    set_target_properties( _${GAFFER_LIB} PROPERTIES PREFIX "" )
  endif()
endforeach()

# Python modules need special naming
if( WIN32 )
  set_target_properties( _Gaffer PROPERTIES SUFFIX ".pyd" )
  set_target_properties( _GafferAppleseed PROPERTIES SUFFIX ".pyd" )
#   set_target_properties( _GafferArnold PROPERTIES SUFFIX ".pyd" )
#   set_target_properties( _GafferCortex PROPERTIES SUFFIX ".pyd" )
  set_target_properties( _GafferDispatch PROPERTIES SUFFIX ".pyd" )
  set_target_properties( _GafferImage PROPERTIES SUFFIX ".pyd" )
  set_target_properties( _GafferImageUI PROPERTIES SUFFIX ".pyd" )
  set_target_properties( _GafferOSL PROPERTIES SUFFIX ".pyd" )
#   set_target_properties( _GafferRenderMan PROPERTIES SUFFIX ".pyd" )
  set_target_properties( _GafferScene PROPERTIES SUFFIX ".pyd" )
  set_target_properties( _GafferSceneUI PROPERTIES SUFFIX ".pyd" )
  set_target_properties( _GafferUI PROPERTIES SUFFIX ".pyd" )
endif()

# Dependencies
#add_dependencies( Gaffer )
add_dependencies( GafferAppleseed Gaffer GafferDispatch GafferScene )
#add_dependencies( GafferArnold Gaffer GafferDispatch GafferScene )
#add_dependencies( GafferCortex Gaffer GafferDispatch )
add_dependencies( GafferDispatch Gaffer )
add_dependencies( GafferImage Gaffer GafferDispatch )
add_dependencies( GafferImageUI Gaffer GafferImage GafferUI GafferBindings GafferImage GafferDispatch GafferImageUI )
add_dependencies( GafferOSL Gaffer GafferScene GafferImage ) 
#add_dependencies( GafferRenderMan Gaffer GafferDispatch GafferScene )
add_dependencies( GafferScene Gaffer GafferImage GafferDispatch )
add_dependencies( GafferSceneUI Gaffer GafferUI GafferImage GafferImageUI GafferScene )
add_dependencies( GafferUI Gaffer )

# Binding Dependencies
add_dependencies( GafferBindings Gaffer )
#add_dependencies( GafferCortexBindings Gaffer GafferBindings GafferCortex GafferDispatch )
add_dependencies( GafferDispatchBindings Gaffer GafferBindings GafferDispatch )
add_dependencies( GafferImageBindings Gaffer GafferBindings GafferImage GafferDispatch )
add_dependencies( GafferImageUIBindings Gaffer GafferBindings GafferImage GafferDispatch )
add_dependencies( GafferSceneBindings Gaffer GafferScene )
add_dependencies( GafferSceneUIBindings Gaffer GafferBindings GafferScene GafferUI GafferImageUI GafferSceneUI )
add_dependencies( GafferUIBindings Gaffer GafferUI GafferBindings )

# Test Dependencies
add_dependencies( GafferTest Gaffer )
add_dependencies( GafferImageTest Gaffer GafferImage )
add_dependencies( GafferSceneTest Gaffer GafferScene GafferDispatch )

# Module dependencies
add_dependencies( _Gaffer Gaffer GafferBindings )
#add_dependencies( _GafferCortex Gaffer GafferBindings GafferCortexBindings )
add_dependencies( _GafferDispatch Gaffer GafferBindings GafferDispatchBindings )
add_dependencies( _GafferImage Gaffer GafferImage GafferBindings GafferImageBindings GafferDispatchBindings )
add_dependencies( _GafferImageUI Gaffer GafferBindings GafferUIBindings GafferImageUIBindings )
add_dependencies( _GafferScene Gaffer GafferBindings GafferSceneBindings GafferDispatchBindings )
add_dependencies( _GafferSceneUI Gaffer GafferBindings GafferUIBindings GafferSceneUIBindings )
add_dependencies( _GafferUI Gaffer GafferBindings GafferUIBindings )
add_dependencies( _GafferOSL GafferBindings GafferScene GafferImage GafferOSL )
add_dependencies( _GafferAppleseed GafferDispatch GafferScene GafferBindings GafferAppleseed )
#add_dependencies( _GafferArnold Gaffer GafferScene GafferBindings GafferDispatch GafferArnold )
#add_dependencies( _GafferRenderMan Gaffer GafferScene GafferBindings GafferDispatch GafferRenderMan )

add_dependencies( _GafferTest Gaffer GafferBindings GafferTest )
add_dependencies( _GafferImageTest Gaffer GafferImage GafferImageTest GafferImageBindings )
add_dependencies( _GafferSceneTest Gaffer GafferBindings GafferScene GafferSceneTest GafferDispatch )

# Install Libs
foreach( GAFFER_LIB ${GAFFER_LIB_NAMES} )
  string( TOUPPER ${GAFFER_LIB} _GAFFER_LIB )
  install( TARGETS ${GAFFER_LIB}
           DESTINATION lib/ )
  install( FILES ${${_GAFFER_LIB}_H_FILES}
           DESTINATION include/${GAFFER_LIB} )
endforeach()

# Install Test Libs
foreach( GAFFER_LIB ${GAFFERTEST_LIB_NAMES} )
  string( TOUPPER ${GAFFER_LIB} _GAFFER_LIB )
  install( TARGETS ${GAFFER_LIB}Test
           DESTINATION lib/ )
  install( FILES ${${_GAFFER_LIB}_H_FILES}
           DESTINATION include/${GAFFER_LIB}Test )
endforeach()

# Install Modules
foreach( GAFFER_LIB ${GAFFER_LIB_NAMES} )
  string( TOUPPER ${GAFFER_LIB} _GAFFER_LIB )
  install( TARGETS _${GAFFER_LIB}
           DESTINATION python/${GAFFER_LIB} )
endforeach()

# Install Bindings
foreach( GAFFER_LIB ${GAFFERBINDINGS_LIB_NAMES} )
  string( TOUPPER ${GAFFER_LIB} _GAFFER_LIB )
  install( TARGETS ${GAFFER_LIB}Bindings 
           DESTINATION lib/ )
  install( FILES ${${_GAFFER_LIB}BINDINGS_H_FILES}
           DESTINATION include/${GAFFER_LIB} )
endforeach()

# Install Gaffer Files
install( DIRECTORY ${CMAKE_SOURCE_DIR}/apps
         DESTINATION . )
install( DIRECTORY ${CMAKE_SOURCE_DIR}/arnold
         DESTINATION . )
install( DIRECTORY ${CMAKE_SOURCE_DIR}/bin
         DESTINATION . )
install( DIRECTORY ${CMAKE_SOURCE_DIR}/glsl
         DESTINATION . )
install( DIRECTORY ${CMAKE_SOURCE_DIR}/python
         DESTINATION . )
install( DIRECTORY ${CMAKE_SOURCE_DIR}/arnold
         DESTINATION . )
install( DIRECTORY ${CMAKE_SOURCE_DIR}/shaders
         DESTINATION . )
install( DIRECTORY ${CMAKE_SOURCE_DIR}/startup
         DESTINATION . )
