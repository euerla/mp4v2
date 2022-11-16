///////////////////////////////////////////////////////////////////////////////
//
//  The contents of this file are subject to the Mozilla Public License
//  Version 1.1 (the "License"); you may not use this file except in
//  compliance with the License. You may obtain a copy of the License at
//  http://www.mozilla.org/MPL/
//
//  Software distributed under the License is distributed on an "AS IS"
//  basis, WITHOUT WARRANTY OF ANY KIND, either express or implied. See the
//  License for the specific language governing rights and limitations
//  under the License.
//
//  The Original Code is MP4v2.
//
//  The Initial Developer of the Original Code is Alexander Heidrich.
//  Portions created by Kona Blend are Copyright (C) 2022.
//  All Rights Reserved.
//
//  Contributors:
//      Alexander Heidrich, euerla@@heidrich.xyz
//
///////////////////////////////////////////////////////////////////////////////

#include "util/impl.h"

using namespace mp4v2::util;

enum Option {
    OPT_HELP              = 'h',
    OPT_VERSION           = 'V',
    OPT_COMPARE           = 'c',
};

static const char* const usageString =
"OPTION... source_file target_file\n"
"Copy all iTunes-compatible tags from MP4 source file to MP4 target file.\n"
"\n"
"      -h, --help      Display this help text and exit\n"
"      -v, --version   Display version information and exit\n"
"      -c, --compare   Copy the tags only if the Contend ID in the source and target are the same";

static const prog::Option long_options[] = {
    { "help",            prog::Option::NO_ARG,       0, OPT_HELP              },
    { "version",         prog::Option::NO_ARG,       0, OPT_VERSION           },
    { "compare",         prog::Option::NO_ARG,       0, OPT_COMPARE           },
    { NULL, prog::Option::NO_ARG, 0, 0 }
};

extern "C" int main( int argc, char** argv ){
    char* progName = argv[0];
    bool compare = false;
    while ( true ) {
        int c = -1;
        int option_index = 0;
        c = prog::getOptionSingle( argc, argv, "h:V", long_options, &option_index );
        if ( c == -1 )
            break;
        switch ( c ) {
            case OPT_HELP:
                fprintf( stderr, "usage: %s %s\n", progName, usageString );
                exit( 0 );
            case OPT_VERSION:
                fprintf( stderr, "%s - %s\n", progName, MP4V2_PROJECT_name_formal );
                exit( 0 );
            case OPT_COMPARE:
                compare = true;
                break;
            default:
                fprintf( stderr, "%s: unknown option specified, ignoring: %c\n", progName, c );
        }
    }
    if ( ( argc - prog::optind ) != 2 ) {
        //fprintf( stderr, "%s: You need to specify two MP4 files as parameters.\n", argv[0] );
        fprintf( stderr, "usage: %s %s\n", progName, usageString );
        exit( 1 );
    }
    printf( "%s version %s\n", progName, MP4V2_PROJECT_version );
    const MP4Tags* sourceTags = MP4TagsAlloc();
    const MP4Tags* targetTags = MP4TagsAlloc();
    char *mp4sourceFileName = argv[prog::optind++];
    printf( "%s:\n", mp4sourceFileName );
    char* sourceInfo = MP4FileInfo( mp4sourceFileName );
    if ( !sourceInfo ) {
        fprintf( stderr,  "%s: can't open %s\n", progName, mp4sourceFileName );
        exit(1);
    }
    MP4FileHandle mp4sourcefile = MP4Read( mp4sourceFileName );
    if ( mp4sourcefile != MP4_INVALID_FILE_HANDLE ) {
        MP4TagsFetch( sourceTags, mp4sourcefile );
        if (compare) {
            fprintf( stdout, "source Content ID: %u\n", *sourceTags->contentID );
        }
        MP4Close( mp4sourcefile );
        free( sourceInfo );
    } else {
        fprintf( stderr,  "%s: the file %s has an invalid file handles\n", progName, mp4sourceFileName );
        exit(1);
    }
    char *mp4destFileName = argv[prog::optind++];
    printf( "%s:\n", mp4destFileName );
    char* destInfo = MP4FileInfo( mp4destFileName );
    if ( !destInfo ) {
        fprintf( stderr,  "%s: can't open %s\n", progName, mp4destFileName );
        exit(1);
    }
    MP4FileHandle mp4destfile = MP4Modify( mp4destFileName);
    if ( mp4destfile != MP4_INVALID_FILE_HANDLE ) {
        MP4TagsFetch( targetTags, mp4destfile );
        if (compare) {
            fprintf( stdout, "target Content ID: %u\n", *targetTags->contentID );
        }
        if (compare && *sourceTags->contentID != *targetTags->contentID) {
            fprintf( stderr, "different content IDs: %u != %u\n", *sourceTags->contentID, *targetTags->contentID );
            exit(1);
        }
        MP4TagsStore( sourceTags, mp4destfile );
        MP4Close( mp4destfile );
        free( destInfo );
    }
    MP4TagsFree( sourceTags );
    MP4TagsFree( targetTags );
    return( 0 );
}
