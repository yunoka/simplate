/*
  +----------------------------------------------------------------------+
  | PHP Version 5                                                        |
  +----------------------------------------------------------------------+
  | Copyright (c) 1997-2004 The PHP Group                                |
  +----------------------------------------------------------------------+
  | This source file is subject to version 3.0 of the PHP license,       |
  | that is bundled with this package in the file LICENSE, and is        |
  | available through the world-wide-web at the following url:           |
  | http://www.php.net/license/3_0.txt.                                  |
  | If you did not receive a copy of the PHP license and are unable to   |
  | obtain it through the world-wide-web, please send a note to          |
  | license@php.net so we can mail you a copy immediately.               |
  +----------------------------------------------------------------------+
  | Author:                                                              |
  +----------------------------------------------------------------------+
*/

/* $Id$ */

#ifndef PHP_SIMPLATE_H
#define PHP_SIMPLATE_H

extern zend_module_entry simplate_module_entry;
#define phpext_simplate_ptr &simplate_module_entry

#ifdef PHP_WIN32
#define PHP_SIMPLATE_API __declspec(dllexport)
#ifndef S_ISDIR
#define S_ISDIR(m) (((m) & _S_IFDIR) == _S_IFDIR)
#endif

#if _MSC_VER >= 1400
#ifndef strdup
#define strdup _strdup
#endif /* strdup */
#ifndef unlink
#define unlink _unlink
#endif
#endif /* _MSC_VER */

#else /* PHP_WIN32 */
#define PHP_SIMPLATE_API
#endif

#ifdef ZTS
#include "TSRM.h"
#endif
//#include <string>

#include <sstream> // stringstream
using std::stringstream;

PHP_MINIT_FUNCTION(simplate);
PHP_MSHUTDOWN_FUNCTION(simplate);
PHP_RINIT_FUNCTION(simplate);
PHP_RSHUTDOWN_FUNCTION(simplate);
PHP_MINFO_FUNCTION(simplate);

#ifdef ZEND_ENGINE_2
ZEND_METHOD(simplate,__construct);
ZEND_METHOD(simplate,assign);
ZEND_METHOD(simplate,fetch);
ZEND_METHOD(simplate,display);
ZEND_METHOD(simplate,clear_cache);
ZEND_METHOD(simplate,register_prefilter);
ZEND_METHOD(simplate,register_postfilter);
#else
PHP_FUNCTION(simplate_init);
PHP_FUNCTION(simplate_assign);
PHP_FUNCTION(simplate_fetch);
PHP_FUNCTION(simplate_display);
PHP_FUNCTION(simplate_clear_cache);
PHP_FUNCTION(simplate_register_prefilter);
PHP_FUNCTION(simplate_register_postfilter);
#endif // ZEND_ENGINE_2

/* 
  	Declare any global variables you may need between the BEGIN
	and END macros here:     
*/
ZEND_BEGIN_MODULE_GLOBALS(simplate)
//	string global_string;
	stringstream global_string; // The C++ string cause segmentation fault in fetch method.
ZEND_END_MODULE_GLOBALS(simplate)

#ifdef _GLOBAL_
ZEND_BEGIN_MODULE_GLOBALS(simplate)
	long  global_value;
	char *global_string;
ZEND_END_MODULE_GLOBALS(simplate)
#endif // _GLOBAL_

/* In every utility function you add that needs to use variables 
   in php_simplate_globals, call TSRMLS_FETCH(); after declaring other 
   variables used by that function, or better yet, pass in TSRMLS_CC
   after the last function argument and declare your utility function
   with TSRMLS_DC after the last declared argument.  Always refer to
   the globals in your function as SIMPLATE_G(variable).  You are 
   encouraged to rename these macros something shorter, see
   examples in any other php module directory.
*/
static int php_my_output_func(const char *str,uint str_len TSRMLS_DC);

#ifdef ZTS
#define SIMPLATE_G(v) TSRMG(simplate_globals_id, zend_simplate_globals *, v)
#else
#define SIMPLATE_G(v) (simplate_globals.v)
#endif

#endif	/* PHP_SIMPLATE_H */


/*
 * Local variables:
 * tab-width: 4
 * c-basic-offset: 4
 * End:
 * vim600: noet sw=4 ts=4 fdm=marker
 * vim<600: noet sw=4 ts=4
 */
