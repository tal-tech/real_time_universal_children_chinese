{
	'targets': [
    {
        'target_name': 'boost_filesystem',
        'type': 'static_library',
        'include_dirs': [
            '..'
        ],
        'sources': [
            'src/filesystem/codecvt_error_category.cpp',
            'src/filesystem/operations.cpp',
            'src/filesystem/path.cpp',
            'src/filesystem/path_traits.cpp',
            'src/filesystem/portability.cpp',
            'src/filesystem/unique_path.cpp',
            'src/filesystem/utf8_codecvt_facet.cpp',
        ],
        'conditions': [
            ['OS=="win"',
            {
                'sources':['src/filesystem/windows_file_codecvt.cpp',]
            }],
        ]
    },
	{
        'target_name': 'boost_thread',
        'type': 'static_library',
        'include_dirs': [
        	'..'
        ],
        'sources': [
        	'src/thread/future.cpp',
        	'src/thread/tss_null.cpp',
        	'src/system/error_code.cpp',
        ],
        #'direct_dependent_settings': {
        #  'defines': [
            #'BOOST_ALL_NO_LIB',
            #'BOOST_THREAD_BUILD_LIB',
        #  ],
        #},
        'conditions': [
        	['OS=="win"',
	        	{
	        		'sources':[
	        			'src/thread/win32/thread.cpp',
	        			'src/thread/win32/tss_dll.cpp',
	        			'src/thread/win32/tss_pe.cpp',
	        		],
	    		},
	    		{
	        		'sources':[
	        			'src/thread/pthread/once.cpp',
	        			'src/thread/pthread/once_atomic.cpp',
	        			'src/thread/pthread/thread.cpp',
	        		],
	    		},
        	],
        ],
    },
	]
}