/*
  +----------------------------------------------------------------------+
  | PHP Version 4/5                                                      |
  +----------------------------------------------------------------------+
  | Copyright (c) Kazuhiro IIzuka All rights reserved.                   |
  +----------------------------------------------------------------------+
  | This source file is the BSD License,                                 |
  | that is bundled with this package in the file LICENSE, and is        |
  | available through the world-wide-web at the following url:           |
  | http://static.aimy.jp/license.txt                                    |
  +----------------------------------------------------------------------+
  | Author: Kazuhiro IIzuka                                              |
  +----------------------------------------------------------------------+
*/

/* $Id$ */

/**
 * @file simplate.cpp
 * @brief Simplate class implementation
 * @author Kazuhiro IIzuka
 * @author Shimizu
 * @version $Id$
 * Copyright (C) Kazuhiro IIzuka
 */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif
#ifdef PHP_WIN32
#include "config.w32.h"
#endif // PHP_WIN32

// C++ header...
#include <iostream>
#include <ctime>
#include <string>
#include <vector>
#include <set>
using std::cout;
using std::cerr;
using std::endl;
using std::string;
using std::vector;
using std::set;

extern "C" {
#include "php.h"
#include "php_ini.h"
#include "ext/standard/info.h"
#include "ext/standard/php_var.h" // var_dump
#ifdef PHP_WIN32
#include "win32/flock.h"
#else
#include "ext/standard/flock_compat.h"
#endif // PHP_WIN32
}
#include "php_simplate.h"

//Debug
#ifdef SIMPLATE_DEBUG
FILE *fp;
char msg[1024];
#ifdef PHP_WIN32
#define DEBUG_PRINTF(format, ...) \
    sprintf(msg, format, __VA_ARGS__); \
    fp = fopen("debug.log", "a"); \
    if (NULL != fp) fprintf(fp, "%s(%d): %s\n", __FUNCTION__, __LINE__, msg); \
    fclose(fp);
#else
#define DEBUG_PRINTF(format, ...) \
    sprintf(msg, format, __VA_ARGS__); \
    fp = fopen("debug.log", "a"); \
    if (NULL != fp) fprintf(fp, "%s(%d): %s\n", __func__, __LINE__, msg); \
    fclose(fp);
#endif // PHP_WIN32
#else
#define DEBUG_PRINTF(format, ...)
#endif // SIMPLATE_DEBUG

//#define PHP5.3 or more
#ifndef Z_REFCOUNT_P

#define Z_REFCOUNT_PP(ppz)              ((*(ppz))->refcount)
#define Z_SET_REFCOUNT_PP(ppz, rc)      ((*(ppz))->refcount = rc)
#define Z_ADDREF_PP(ppz)                (++(*(ppz))->refcount)
#define Z_DELREF_PP(ppz)                (--(*(ppz))->refcount)
#define Z_ISREF_PP(ppz)                 ((*(ppz))->is_ref)
#define Z_SET_ISREF_PP(ppz)             ((*(ppz))->is_ref = 1)
#define Z_UNSET_ISREF_PP(ppz)           ((*(ppz))->is_ref = 0)
#define Z_SET_ISREF_TO_PP(ppz, isref)   ((*(ppz))->is_ref = isref)

#define Z_REFCOUNT_P(pz)                ((pz)->refcount)
#define Z_SET_REFCOUNT_P(z, rc)         ((pz)->refcount = rc)
#define Z_ADDREF_P(pz)                  (++(pz)->refcount)
#define Z_DELREF_P(pz)                  (--(pz)->refcount)
#define Z_ISREF_P(pz)                   ((pz)->is_ref)
#define Z_SET_ISREF_P(pz)               ((pz)->is_ref = 1)
#define Z_UNSET_ISREF_P(pz)             ((pz)->is_ref = 0)
#define Z_SET_ISREF_TO_P(z, isref)      ((pz)->is_ref = isref)

#define Z_REFCOUNT(z)                   ((z).refcount)
#define Z_SET_REFCOUNT(z, rc)           ((z).refcount = rc)
#define Z_ADDREF(z)                     (++(z).refcount)
#define Z_DELREF(z)                     (--(z).refcount)
#define Z_ISREF(z)                      ((z).is_ref)
#define Z_SET_ISREF(z)                  ((z).is_ref = 1)
#define Z_UNSET_ISREF(z)                ((z).is_ref = 0)
#define Z_SET_ISREF_TO(z, isref)        ((z).is_ref = isref)

#endif

//#define USE_ZEND_EXECUTE
#define SET_ZVAL_STRING(z, s) \
    INIT_PZVAL(&z); \
    Z_STRVAL(z) = estrndup(s, strlen(s)); \
    Z_STRLEN(z) = strlen(s); \
    Z_TYPE(z) = IS_STRING;

/* If you declare any globals in php_simplate.h uncomment this: */
ZEND_DECLARE_MODULE_GLOBALS(simplate)

/* True global resources - no need for thread safety here */
static int le_simplate;

// default parameters
#define DEFAULT_TEMPLATE_DIR "template"
#define DEFAULT_COMPILE_DIR "template_c"
#define DEFAULT_CACHE_DIR "cache"
#define DEFAULT_LEFT_DELIMITER "<{"
#define DEFAULT_RIGHT_DELIMITER "}>"
#define DEFAULT_COMPILE_CHECK true
#define DEFAULT_FORCE_COMPILE false
#define DEFAULT_LAZY_CHECK false
#define DEFAULT_CACHE_LIFETIME 3600
#define DEFAULT_CACHING 0 // 0: no caching
                          // 1: use class cache_lifetime value
                          // 2: use cache_lifetime in cache file

// fetch mode
#define SIMPLATE_FETCH 0
#define SIMPLATE_DISPLAY 1

// parameter names
#define TEMPLATE_DIR "template_dir"
#define COMPILE_DIR "compile_dir"
#define CACHE_DIR "cache_dir"
#define LEFT_DELIMITER "left_delimiter"
#define RIGHT_DELIMITER "right_delimiter"
#define COMPILE_CHECK "compile_check"
#define FORCE_COMPILE "force_compile"
#define LAZY_CHECK "lazy_check"
#define VERSION "0.4.2"
#define CACHE_LIFETIME "cache_lifetime"
#define CACHING "caching"

// class entry pointer
static zend_class_entry *simplate_entry_ptr;
static zend_function_entry php_simplate_functions[] = {
#ifdef ZEND_ENGINE_2
  ZEND_ME(simplate, __construct, NULL, ZEND_ACC_PUBLIC)
  ZEND_ME(simplate, assign, NULL, ZEND_ACC_PUBLIC)
  ZEND_ME(simplate, fetch, NULL, ZEND_ACC_PUBLIC)
  ZEND_ME(simplate, display, NULL, ZEND_ACC_PUBLIC)
  ZEND_ME(simplate, clear_cache, NULL, ZEND_ACC_PUBLIC)
  ZEND_ME(simplate, register_prefilter, NULL, ZEND_ACC_PUBLIC)
  ZEND_ME(simplate, register_postfilter, NULL, ZEND_ACC_PUBLIC)
#else
  PHP_FALIAS(simplate, simplate_init, NULL)
  PHP_FALIAS(assign, simplate_assign, NULL)
  PHP_FALIAS(fetch, simplate_fetch, NULL)
  PHP_FALIAS(display, simplate_display, NULL)
  PHP_FALIAS(clear_cache, simplate_clear_cache, NULL)
  PHP_FALIAS(register_prefilter, simplate_register_prefilter, NULL)
  PHP_FALIAS(register_postfilter, simplate_register_postfilter, NULL)
#endif // ZEND_ENGINE_2
  {NULL, NULL, NULL}
};

/* {{{ simplate_functions[]
 *
 * Every user visible function must have an entry in simplate_functions[].
 */
function_entry simplate_functions[] = {
    {NULL, NULL, NULL} /* Must be the last line in simplate_functions[] */
};
/* }}} */

/* {{{ simplate_module_entry
 */
zend_module_entry simplate_module_entry = {
#if ZEND_MODULE_API_NO >= 20010901
    STANDARD_MODULE_HEADER,
#endif
    "simplate",
    simplate_functions,
    PHP_MINIT(simplate),
    PHP_MSHUTDOWN(simplate),
    PHP_RINIT(simplate), /* Replace with NULL if there's nothing to do at request start */
    PHP_RSHUTDOWN(simplate), /* Replace with NULL if there's nothing to do at request end */
    PHP_MINFO(simplate),
#if ZEND_MODULE_API_NO >= 20010901
    VERSION,
#endif
    STANDARD_MODULE_PROPERTIES
};
/* }}} */

#ifdef COMPILE_DL_SIMPLATE
BEGIN_EXTERN_C()
ZEND_GET_MODULE(simplate)
END_EXTERN_C()
#endif

/* {{{ PHP_INI
 */
/* Remove comments and fill if you need to have entries in php.ini
PHP_INI_BEGIN()
    STD_PHP_INI_ENTRY("simplate.global_value",      "42", PHP_INI_ALL, OnUpdateLong, global_value, zend_simplate_globals, simplate_globals)
    STD_PHP_INI_ENTRY("simplate.global_string", "foobar", PHP_INI_ALL, OnUpdateString, global_string, zend_simplate_globals, simplate_globals)
PHP_INI_END()
*/
/* }}} */

/* {{{ php_simplate_init_globals
 */
/* Uncomment this function if you have INI entries */
static void php_simplate_init_globals(
    zend_simplate_globals *simplate_globals TSRMLS_DC
)
{
    simplate_globals->global_string;
}
/* }}} */

#if 0
// create custom objects
typedef struct{
    string fetch_buffer;
    zend_object zo;
}simplate_object;

static void simplate_object_dtor(void *object, zend_object_handle handle TSRMLS_DC)
{
    zend_objects_destroy_object(object, handle TSRMLS_CC);
}
static void simplate_objects_clone(void *object, void **object_clone TSRMLS_DC)
{
    simplate_object *intern = (simplate_object*)object;
    simplate_object **intern_clone = (simplate_object**)object_clone;

    *intern_clone = emalloc(sizeof(simplate_object));
    (*intern_object)->zo.ce = intern->zo.ce;
    (*intern_object)->zo.in_get = 0;
    (*intern_object)->zo.in_set = 0;
    ALLOC_HASHTABLE((*intern_clone)->zo.properties);
    (*intern_clone)->fetch_buffer=

}
#endif // 0

/* {{{ Returns full path of the specified filename.
 *
 * ex)
 *   filename = "foo.tpl"
 *     => template_dir + DEFAULT_SLASH + "foo.tpl"
 *   filename = "$variable"
 *     => template_dir + DEFAULT_SLASH + $variable
 *   filename = "$variable.tpl"
 *     => template_dir + DEFAULT_SLASH + $variable ".tpl"
 *   filename = "/path/to/foo.tpl"
 *     => "/path/to/foo.tpl"
 *
 * @param zval   *obj         instance of this class
 * @param string template_dir template directory
 * @param string filename     filename
 *
 * @return string full path of the specified filename
 */
static string get_include_filename(
    zval   *obj,
    string template_dir,
    string filename TSRMLS_DC
)
{
    string included_filename; // return value

    // search "$" (use variable or don't use variable)
    size_t pos = filename.find("$");
    if (pos != string::npos) {
        // use variable
        string include_variable = "";

        // From "$" to "." string is cut out.
        //   $variable => $variable
        //   $variable.tpl => $variable
        include_variable = filename.substr(pos);
        size_t dot_pos = include_variable.find(".");
        if (dot_pos != string::npos) {
            include_variable = include_variable.substr(0, dot_pos);
        }

DEBUG_PRINTF("[B2]include_variable = (%s)", const_cast<char*>(include_variable.c_str()));

        // delete "$"
        //   $variable => variable
        include_variable.erase(0, 1);

#ifdef ZEND_ENGINE_2
        zval *_tpl_vars = zend_read_property(Z_OBJCE_P(obj), obj, const_cast<char*>("_tpl_vars"), strlen("_tpl_vars"), 1 TSRMLS_CC);
#else
        zval *_tpl_vars;
        if (zend_hash_find(Z_OBJPROP_P(obj), const_cast<char*>("_tpl_vars"), sizeof("_tpl_vars"), (void**)&_tpl_vars) != SUCCESS) {
            zend_error(E_ERROR, "_tpl_vars not found from zend_hash.");
        }
#endif // ZEND_ENGINE_2
        zval **zinclude_file;
        string new_filename;
        if (zend_hash_find(Z_ARRVAL_P(_tpl_vars), const_cast<char*>(include_variable.c_str()), include_variable.length() + 1, (void**)&zinclude_file) == SUCCESS
            && Z_TYPE_PP(zinclude_file) == IS_STRING
        ) {
DEBUG_PRINTF("(%s) -> (%s)", const_cast<char*>(include_variable.c_str()), Z_STRVAL_PP(zinclude_file));
            new_filename = filename.replace(filename.find("$"), include_variable.length() + 1, Z_STRVAL_PP(zinclude_file));
        } else {
            zend_error(E_ERROR, "include file variable = ($%s) is not assigned.", include_variable.c_str());
        }
        if (new_filename[0] == DEFAULT_SLASH) {
            // use variable
            // $variable = "/path/to/file.tpl"
            included_filename = new_filename;
        } else {
            included_filename = template_dir + DEFAULT_SLASH + new_filename;
        }
    } else if (filename[0] == DEFAULT_SLASH) {
        // don't use variable
        // <{include file="/path/to/file.tpl"}>
        included_filename = filename;
    } else {
        // don't use variable
        // <{include file="file.tpl"}>
        included_filename = template_dir + DEFAULT_SLASH + filename;
    }
    return included_filename;
}
/* }}} */

/* {{{ Read contents from the specified filename, and replaces xml start-end tags.
 *
 *   <?xml
 *     => <?php echo'<?xml'; ?>
 *   ?>
 *     => <?php echo'?>'; ?>
 *
 * @param const char *filename filename
 *
 * @return string contents
 */
static string _readfile(
    const char *filename TSRMLS_DC
)
{
    string line;

    // check open_basedir restriction in php.ini
    if (php_check_open_basedir(filename TSRMLS_CC)) {
        return "";
    }

#ifdef PHP_WIN32
    php_stream *strm;
    char *buffer = NULL;
    size_t buffer_length = 0;

    strm = php_stream_open_wrapper(const_cast<char*>(filename), "rb", ENFORCE_SAFE_MODE | REPORT_ERRORS, NULL);
    if (!strm) {
        zend_error(E_ERROR, "Cannot read such file or directory:%s", const_cast<char*>(filename));
        return "";
    }

    buffer_length = php_stream_copy_to_mem(strm, &buffer, PHP_STREAM_COPY_ALL, 0);
    php_stream_close(strm);
    if (buffer_length < 0) {
        zend_error(E_ERROR, "Cannot read such file or directory:%s", const_cast<char*>(filename));
        efree(buffer);
        return "";
    }
    line = buffer;
    efree(buffer);
#else // PHP_WIN32
    FILE *fp;
    char buf[8192];
    fp = VCWD_FOPEN(filename, "rb");
    if (fp == NULL) {
        zend_error(E_ERROR, "Cannot read such file or directory:%s", const_cast<char*>(filename));
        return "";
    }

    while (fgets(buf, sizeof(buf), fp) != 0) {
        line += buf;
    }
    fclose(fp);
#endif // PHP_WIN32

    // replace xml tag
    //   <?xml => <?php echo'<?xml'; ?>
    //   ?>    => <?php echo'?>'; ?>
    const char *xml_tag = "<?xml";
    size_t pos = 0;
    string new_tag;
    while ((pos = line.find(xml_tag, pos)) != string::npos) {
        new_tag = "<?php echo \'";
        new_tag += xml_tag;
        new_tag += "\'; ?>";
        line.replace(pos, strlen(xml_tag), new_tag);
        pos += new_tag.length();

        new_tag = "<?php echo \'?>\'; ?>";
        pos = line.find("?>", pos);
        line.replace(pos, 2, new_tag);
        pos += new_tag.length();
    }

    return line;
}
/* }}} */

/* {{{ Trims the specified string.
 *
 * @param const char *s string
 *
 * @return string trim string
 */
static string trim(
    const char *s
)
{
    string new_str(s);
    // trim from left-side
    while (new_str[0] == ' ') {
        new_str.erase(0, 1);
    }
    // trim from right-side
    while (new_str[new_str.length() - 1] == ' ') {
        new_str.erase(new_str.length() - 1, 1);
    }
    return new_str;
}
/* }}} */

/* {{{ Splits the specified string with the separator. The results is stored in the specified vector.
 *
 * @param const char     *s        string
 * @param vector<string> &e        vector
 * @param char           separator separator (= ".")
 *
 * @return void
 */
static void split_element(
    const char     *s,
    vector<string> &e,
    char           separator = '.'
)
{
    string str(s);
    size_t spos = 0, epos = 0;
    e.push_back(str.substr(spos, str.find(separator, spos)));
    while ((epos = str.find(separator, epos)) != string::npos) {
        e.push_back(str.substr(epos + 1, str.find(separator, epos + 1) - epos - 1));
        epos++;
    }
}
/* }}} */

/* {{{ Looks for [a-z0-9_] consecutive string from i position of the specified variable. The result is stored in the specified new_variable.
 *
 * @param const char *variable
 * @param int        i
 * @param string     &new_variable
 *
 * @return int length of [a-z0-9_] consecutive string
 */
static int get_identifier(
    const char *variable,
    int        i,
    string     &new_variable
)
{
    int j = 0;
    while (isalnum(variable[i]) || variable[i] == '_') {
        new_variable += variable[i];
        i++;
        j++;
    }
    return j;
}
/* }}} */

/* {{{ Parses the specified variable.
 *
 * array pattern
 *   $hoge[i] => _tpl_vars['hoge'][$i]
 *   $hoge[i].foo.bar => _tpl_vars['hoge'][$i]['foo']['bar']
 *   $hoge[i].huga[j].foo => _tpl_vars['hoge'][$i]['huga'][$j]['foo']
 *   $hoge[i]->huga[j]->foo => _tpl_vars['hoge'][$i]->huga[$j]->foo
 *
 * section pattern
 *   $simplate.section.i.index => $i
 *   $simplate.section.ary.count => count($this->_tpl_vars['ary'])
 *   $simplate.section.i.first => $this->_section['i']['first']
 *   $simplate.section.i.last => $this->_section['i']['last']
 *
 * arrow pattern
 *   $ary10.foo->bar => $this->_tpl_vars['ary10']['foo']->bar
 *   $foo->bar => $this->_tpl_vars['foo']->bar
 *
 * normal pattern
 *   $key => $this->_tpl_vars['key']
 *
 * @param const char *variable variable string
 *
 * @return string parsed variable string
 */
static string parse_variable(
    const char *variable
)
{
    string new_variable = "";

DEBUG_PRINTF("variable = (%s)", variable);

    if (variable[0] != '$' || strncmp(variable, "$this", 5) == 0) {
        new_variable = variable;

DEBUG_PRINTF("new_variable = (%s)", const_cast<char*>(new_variable.c_str()));

        return new_variable;
    }

    if (strstr(variable, "[")) {

DEBUG_PRINTF("%s", "array pattern");

        // $hoge[i] => _tpl_vars['hoge'][$i]
        // $hoge[i].foo.bar => _tpl_vars['hoge'][$i]['foo']['bar']
        // $hoge[i].huga[j].foo => _tpl_vars['hoge'][$i]['huga'][$j]['foo']
        // $hoge[i]->huga[j]->foo => _tpl_vars['hoge'][$i]->huga[$j]->foo

        int i = 0;
        while (variable[i]) {
            if (variable[i] == '$') {
                new_variable += "$this->_tpl_vars['";
                i++;
                i += get_identifier(variable, i, new_variable);
                new_variable += "']";
            } else if (variable[i] == '.') {
                new_variable += "['";
                i++;
                i += get_identifier(variable, i, new_variable);
                new_variable += "']";
            } else if (variable[i] == '[') {
                new_variable += "[$";
                i++;
                i += get_identifier(variable, i, new_variable);
                new_variable += "]";
                i++;
            } else if (i > 0 && variable[i] == '>' && variable[i - 1] == '-') {
                new_variable += "->";
                i++;
                i += get_identifier(variable, i, new_variable);
            } else {
                i++;
            }
        }
    } else if (strstr(variable, ".")) {

DEBUG_PRINTF("%s", "section pattern");

        vector<string> elements;
        split_element(variable + 1, elements);

#ifdef SIMPLATE_DEBUG
for (vector<string>::iterator i = elements.begin(); i != elements.end(); ++i) {
DEBUG_PRINTF("vector(i) = (%s)", const_cast<char*>((*i).c_str()));
}
#endif // SIMPLATE_DEBUG

        // $simplate.section.i.index => $i
        // $simplate.section.ary.count => count($this->_tpl_vars['ary'])
        // $simplate.section.i.first => $this->_section['i']['first']
        // $simplate.section.i.last => $this->_section['i']['last']
        // $ary10.foo->bar => $this->_tpl_vars["ary10"]["foo"]->bar
        if (elements.size() == 4 && elements[1] == "section" && elements[3] == "index") {
            new_variable += "$";
            new_variable += elements[2];
        } else if (elements.size() == 4 && elements[1] == "section" && elements[3] == "count") {
            new_variable += "count($this->_tpl_vars['";
            new_variable += elements[2];
            new_variable += "'])";
        } else if (elements.size() == 4 && elements[1] == "section" && (elements[3] == "first"|| elements[3] == "last")) {
            new_variable += "$this->_section['";
            new_variable += elements[2];
            new_variable += "']['";
            new_variable += elements[3];
            new_variable += "']";
        } else {
            new_variable += "$this->_tpl_vars";
            for (vector<string>::iterator i = elements.begin(); i != elements.end(); ++i) {
                new_variable += "['";

                size_t pos = 0;
                if ((pos = (*i).find("->")) != string::npos) {
                    new_variable += (*i).substr(0, pos);
                    new_variable += "']";
                    new_variable += (*i).substr(pos);
                } else {
                    new_variable += *i;
                    new_variable += "']";
                }
            }
        }
    } else if (strstr(variable, "->")) {

DEBUG_PRINTF("%s", "arrow pattern");

        // $foo->bar => $this->_tpl_vars['foo']->bar
        string temp = variable;
        new_variable += "$this->_tpl_vars['";
        new_variable += temp.substr(1, temp.find("->") - 1);
        new_variable += "']";
        new_variable += temp.substr(temp.find("->"));
    } else {

DEBUG_PRINTF("%s", "normal pattern");

        // $key => $this->_tpl_vars['key']
        new_variable += "$this->_tpl_vars['";
        new_variable += (variable + 1);
        new_variable += "']";
    }

DEBUG_PRINTF("new_variable = (%s)", const_cast<char*>(new_variable.c_str()));

    return new_variable;
}
/* }}} */

/* {{{ Returns the value of the simplate tag's attribute to which specified key is coinciding
 *
 * @param const char *s   simplate tag
 * @param const char *key attribute name
 *
 * @return string the value of simplate tag's attribute
 */
static string get_element(
    const char *s,
    const char *key
)
{
    int i;
    string value = "";
    string key_name;

DEBUG_PRINTF("s = (%s), key = (%s)", s, key);

    while ((s = strstr(s, key)) != NULL) {
        i = 0;
        key_name = "";
        while (isalnum(*(s + i))) {
            key_name += *(s + i);
            i++;
        }
        if (strcmp(key_name.c_str(), key) == 0) {
            while (*(s + i) != '=') {
                i++;
            }
            i++;
            while (*(s + i) == '\'' || *(s + i) == '"' || *(s + i) == ' ') {
                i++;
            }
            while (*(s + i) != '\'' && *(s + i) != '"' && *(s + i) != ' ' && *(s + i)) {
                value += *(s + i);
                i++;
            }
            break;
        }
        s++;
    }

DEBUG_PRINTF("value = (%s)", const_cast<char*>(value.c_str()));

    return value;
}
/* }}} */

/* {{{ Returns true if the specified file has compiled.
 *
 * @param zval       *obj             instance of this
 * @param const char *template_dir    template directory
 * @param const char *compile_dir     compile direcotry
 * @param const char *filename        target file
 * @param const char *left_delimiter  left-delimiter
 * @param const char *right_delimiter right-delimiter
 *
 * @return boolean returns true if the specified file has compiled
 */
static inline bool _is_compiled(
    zval       *obj,
    const char *template_dir,
    const char *compile_dir,
    const char *filename,
    const char *left_delimiter,
    const char *right_delimiter TSRMLS_DC
)
{
    string error;
    unsigned char compile_check = DEFAULT_COMPILE_CHECK;
    unsigned char force_compile = 0;
    unsigned char lazy_check = 0;

#ifdef ZEND_ENGINE_2
    zval *temp_zval;

    // Reads property COMPILE_CHECK
    temp_zval = zend_read_property(Z_OBJCE_P(obj), obj, const_cast<char*>(COMPILE_CHECK), strlen(COMPILE_CHECK), 1 TSRMLS_CC);
    if (Z_TYPE_P(temp_zval) == IS_BOOL) {
        compile_check = Z_BVAL_P(temp_zval);
    }

    // Reads property FORCE_COMPILE
    temp_zval = zend_read_property(Z_OBJCE_P(obj), obj, const_cast<char*>(FORCE_COMPILE), strlen(FORCE_COMPILE), 1 TSRMLS_CC);
    if (Z_TYPE_P(temp_zval) == IS_BOOL) {
        force_compile = Z_BVAL_P(temp_zval);
    }

    // Reads property LAZY_CHECK
    temp_zval = zend_read_property(Z_OBJCE_P(obj), obj, const_cast<char*>(LAZY_CHECK), strlen(LAZY_CHECK), 1 TSRMLS_CC);
    if (Z_TYPE_P(temp_zval) == IS_BOOL) {
        lazy_check = Z_BVAL_P(temp_zval);
    }
#else
    zval **temp;

    // Reads property COMPILE_CHECK
    if (zend_hash_find(Z_OBJPROP_P(obj), const_cast<char*>(COMPILE_CHECK), sizeof(COMPILE_CHECK), (void**)&temp) == SUCCESS) {
        if (Z_TYPE_PP(temp) == IS_BOOL) {
            compile_check = Z_BVAL_PP(temp);
        }
    }

    // Reads property FORCE_COMPILE
    if (zend_hash_find(Z_OBJPROP_P(obj), const_cast<char*>(FORCE_COMPILE), sizeof(FORCE_COMPILE), (void**)&temp) == SUCCESS) {
        if (Z_TYPE_PP(temp) == IS_BOOL) {
            force_compile = Z_BVAL_PP(temp);
        }
    }

    // Reads property LAZY_CHECK
    if (zend_hash_find(Z_OBJPROP_P(obj), const_cast<char*>(LAZY_CHECK), sizeof(LAZY_CHECK), (void**)&temp) == SUCCESS) {
        if (Z_TYPE_PP(temp) == IS_BOOL) {
            lazy_check = Z_BVAL_PP(temp);
        }
    }
#endif // ZEND_ENGINE_2

DEBUG_PRINTF("compile_check = (%d)", compile_check);
DEBUG_PRINTF("force_compile = (%d)", force_compile);
DEBUG_PRINTF("lazy_check = (%d)", lazy_check);

    // Returns false if force_compile is true.
    if (force_compile) {
        return false;
    }

    // Has the compiled file already exists?
    struct stat cs;
    string full_compile_filename(compile_dir);
    full_compile_filename += DEFAULT_SLASH;
    full_compile_filename += filename;
    if (VCWD_STAT(full_compile_filename.c_str(), &cs) == -1) {
        // the compiled file doesn't exists.
        return false;
    }

    // Returns true if compile_check is false.
    if (!compile_check) {
        return true;
    }

    // compare the file and compiled file.
    struct stat ts;
    string full_template_filename(template_dir);
    full_template_filename += DEFAULT_SLASH;
    full_template_filename += filename;

    if (VCWD_STAT(full_template_filename.c_str(), &ts) == -1) {
        zend_error(E_ERROR, "cannot stat:%s", full_template_filename.c_str());
        return false;
    }

DEBUG_PRINTF("template_filename = (%s)", full_template_filename.c_str());
DEBUG_PRINTF("compiled_filename = (%s)", full_compile_filename.c_str());
DEBUG_PRINTF("cs.st_mtime = (%ld), ts.st_mtime = (%ld)", cs.st_mtime, ts.st_mtime);

    // Returns false if template file is newer than compiled file.
    if (cs.st_mtime < ts.st_mtime) {
        return false;
    } else {
        // Not checks the included file if lazy check is true.
        if (!lazy_check) {

            size_t pos = 0;
            size_t tag_start_pos, tag_end_pos;
            size_t element_start_pos, element_end_pos;
            string item;
            string file_content = _readfile(full_template_filename.c_str() TSRMLS_CC);

            // Reads include tags.
            while ((pos = file_content.find(left_delimiter, pos)) != string::npos) {
                tag_start_pos = pos;
                element_start_pos = tag_start_pos + strlen(left_delimiter);
                element_end_pos = file_content.find(right_delimiter, tag_start_pos);
                if (element_end_pos == string::npos) {
                    zend_error(E_ERROR, "No closed delimiter:`%s' in %s", right_delimiter, full_template_filename.c_str());
                    return false;
                }
                tag_end_pos = element_end_pos + strlen(right_delimiter);

                // Reads between left_delimiter and right_delimiter.
                item.assign(file_content, element_start_pos, element_end_pos - element_start_pos);
                item = trim(item.c_str());

                // Finds include tag.
                if (item.substr(0, 7) == "include") {

DEBUG_PRINTF("item = (%s)", item.c_str());

                    string filename = get_element(item.c_str(), "file");
                    string included_filename = get_include_filename(obj, template_dir, filename TSRMLS_CC);

                    // Check included template file timestamp
                    struct stat is;
                    if (VCWD_STAT(included_filename.c_str(), &is) == -1) {
                        zend_error(E_ERROR, "cannot stat:%s", included_filename.c_str());
                        return false;
                    }

DEBUG_PRINTF("included_filename = (%s)", included_filename.c_str());
DEBUG_PRINTF("cs.st_mtime = (%ld), is.st_mtime = (%ld)", cs.st_mtime, is.st_mtime);
                    // Returns false if included file is newer than compiled file.
                    if (cs.st_mtime < is.st_mtime) {
                        return false;
                    }

                    // Reads included file.
                    string new_condition = _readfile(included_filename.c_str() TSRMLS_CC);

                    // Needs this check ?
                    if (new_condition.find(filename) != string::npos) {
                        zend_error(E_ERROR, "Found recursive include:%s", filename.c_str());
                        return "";
                    }
                    file_content.replace(tag_start_pos, tag_end_pos - tag_start_pos, new_condition);
                    continue;
                }
                pos++;
            }
        } // lazy check
    }

    return true;
}
/* }}} */

/* {{{ Parses and compiles the specified temaplte file.
 *
 * @param INTERNAL_FUNCTION_PARAMETERS see zend.h
 * @param char **fullfile_name         returns full path of template file
 * @param int  mode                    SIMPLATE_FETCH or SIMPLATE_DISPLAY
 * @param char **cache_content         returns results (use fetch mode)
 *
 * @return void
 */
void read_parse_template(
    INTERNAL_FUNCTION_PARAMETERS,
    char **fullfile_name,
    int mode,
    char **cache_content = NULL
)
{
    zval *obj;
    char *resource_name = NULL;
    int resource_name_len = 0;
    string template_dir;
    char *compile_dir = NULL;
    char *left_delimiter = NULL;
    char *right_delimiter = NULL;
    long caching = 0;
    long cache_lifetime = 0;
    size_t i = 0;
    string error;

    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, const_cast<char*>("s"), &resource_name, &resource_name_len) == FAILURE) {
        return;
    }

DEBUG_PRINTF("resource_name = (%s)", resource_name);

    obj = getThis();
#ifdef ZEND_ENGINE_2
    zval *temp_zval;
    temp_zval = zend_read_property(Z_OBJCE_P(obj), obj, const_cast<char*>(TEMPLATE_DIR), strlen(TEMPLATE_DIR), 1 TSRMLS_CC);
    if (Z_TYPE_P(temp_zval) == IS_STRING) {
        template_dir = Z_STRVAL_P(temp_zval);
    }
    temp_zval = zend_read_property(Z_OBJCE_P(obj), obj, const_cast<char*>(COMPILE_DIR), strlen(COMPILE_DIR), 1 TSRMLS_CC);
    if (Z_TYPE_P(temp_zval) == IS_STRING) {
        compile_dir = Z_STRVAL_P(temp_zval);
    }
    temp_zval = zend_read_property(Z_OBJCE_P(obj), obj, const_cast<char*>(LEFT_DELIMITER), strlen(LEFT_DELIMITER), 1 TSRMLS_CC);
    if (Z_TYPE_P(temp_zval) == IS_STRING) {
        left_delimiter = Z_STRVAL_P(temp_zval);
    }
    temp_zval = zend_read_property(Z_OBJCE_P(obj), obj, const_cast<char*>(RIGHT_DELIMITER), strlen(RIGHT_DELIMITER), 1 TSRMLS_CC);
    if (Z_TYPE_P(temp_zval) == IS_STRING) {
        right_delimiter = Z_STRVAL_P(temp_zval);
    }
    temp_zval = zend_read_property(Z_OBJCE_P(obj), obj, const_cast<char*>(CACHING), strlen(CACHING), 1 TSRMLS_CC);
    if (Z_TYPE_P(temp_zval) == IS_LONG) {
        caching = Z_LVAL_P(temp_zval);
    }
    temp_zval = zend_read_property(Z_OBJCE_P(obj), obj, const_cast<char*>(CACHE_LIFETIME), strlen(CACHE_LIFETIME), 1 TSRMLS_CC);
    if (Z_TYPE_P(temp_zval) == IS_LONG) {
        cache_lifetime = Z_LVAL_P(temp_zval);
    }
#else
    zval **temp_zval;

    // Logic to implement var_dump($this) for PHP4
    // $this = &$smarty;
    // zend_hash_update(&EG(symbol_table), "this", strlen("this") + 1, &obj, sizeof(zval*), NULL);

    if (zend_hash_find(Z_OBJPROP_P(obj), TEMPLATE_DIR, sizeof(TEMPLATE_DIR), (void**)&temp_zval) == SUCCESS) {
        if ((*temp_zval)->type == IS_STRING) {
          template_dir = Z_STRVAL_PP(temp_zval);
        }
    }
    if (zend_hash_find(Z_OBJPROP_P(obj), COMPILE_DIR, sizeof(COMPILE_DIR), (void**)&temp_zval) == SUCCESS) {
        if ((*temp_zval)->type == IS_STRING) {
            compile_dir = Z_STRVAL_PP(temp_zval);
        }
    }
    if (zend_hash_find(Z_OBJPROP_P(obj), LEFT_DELIMITER, sizeof(LEFT_DELIMITER), (void**)&temp_zval) == SUCCESS) {
        if ((*temp_zval)->type == IS_STRING) {
            left_delimiter = Z_STRVAL_PP(temp_zval);
        }
    }
    if (zend_hash_find(Z_OBJPROP_P(obj), RIGHT_DELIMITER, sizeof(RIGHT_DELIMITER), (void**)&temp_zval) == SUCCESS) {
        if ((*temp_zval)->type == IS_STRING) {
            right_delimiter = Z_STRVAL_PP(temp_zval);
        }
    }
    if (zend_hash_find(Z_OBJPROP_P(obj), CACHING, sizeof(CACHING), (void**)&temp_zval) == SUCCESS) {
        if ((*temp_zval)->type == IS_LONG) {
            caching = Z_LVAL_PP(temp_zval);
        }
    }
    if (zend_hash_find(Z_OBJPROP_P(obj), CACHE_LIFETIME, sizeof(CACHE_LIFETIME), (void**)&temp_zval) == SUCCESS) {
        if ((*temp_zval)->type == IS_LONG) {
            cache_lifetime = Z_LVAL_PP(temp_zval);
        }
    }
#endif // ZEND_ENGINE_2

DEBUG_PRINTF("template_dir = (%s)", template_dir.c_str());
DEBUG_PRINTF("compile_dir = (%s)", compile_dir);
DEBUG_PRINTF("left_delimiter = (%s)", left_delimiter);
DEBUG_PRINTF("right_delimiter = (%s)", right_delimiter);
DEBUG_PRINTF("caching = (%ld)", caching);
DEBUG_PRINTF("cache_lifetime = (%ld)", cache_lifetime);

    if (template_dir[template_dir.length() - 1] == DEFAULT_SLASH) {
        template_dir.erase(template_dir.length() - 1, 1);
    }
    string full_template_filename(template_dir);
    full_template_filename += DEFAULT_SLASH;
    full_template_filename += resource_name;

DEBUG_PRINTF("full_template_filename = (%s)", full_template_filename.c_str());

    string full_compile_filename(compile_dir);
    if (full_compile_filename[full_compile_filename.length() - 1] != DEFAULT_SLASH) {
        full_compile_filename += DEFAULT_SLASH;
    }
    full_compile_filename += resource_name;

DEBUG_PRINTF("full_compile_filename = (%s)", full_compile_filename.c_str());

    if (caching) {
        struct stat cache_stat;
        char *cache_dir;
        unsigned char compile_check;
        unsigned char force_compile;
#ifdef ZEND_ENGINE_2
        cache_dir = Z_STRVAL_P(zend_read_property(Z_OBJCE_P(obj), obj, const_cast<char*>(CACHE_DIR), strlen(CACHE_DIR), 1 TSRMLS_CC));
        compile_check = Z_BVAL_P(zend_read_property(Z_OBJCE_P(obj), obj, const_cast<char*>(COMPILE_CHECK), strlen(COMPILE_CHECK), 1 TSRMLS_CC));
        force_compile = Z_BVAL_P(zend_read_property(Z_OBJCE_P(obj), obj, const_cast<char*>(FORCE_COMPILE), strlen(FORCE_COMPILE), 1 TSRMLS_CC));
#else
#endif // ZEND_ENGINE_2
        if (cache_dir[strlen(cache_dir) - 1] == DEFAULT_SLASH) {
            cache_dir[strlen(cache_dir) - 1] = '\0';
        }
        // cache directory exists?
        if (VCWD_STAT(cache_dir, &cache_stat) != -1) {
            if (!S_ISDIR(cache_stat.st_mode)) {
                zend_error(E_ERROR, "does not exist cache directory: %s", cache_dir);
                return;
            }
        }
        string full_cache_filename(cache_dir);
        full_cache_filename += DEFAULT_SLASH;
        full_cache_filename += resource_name;

DEBUG_PRINTF("full_cache_filename = (%s)", full_cache_filename.c_str());

        // there are no cache file.
        if (VCWD_STAT(full_cache_filename.c_str(), &cache_stat) == -1 || force_compile) {

create_cache:
            if (VCWD_STAT(full_compile_filename.c_str(), &cache_stat) == -1) {
                goto compile;
            }

            zend_file_handle file_handle;
            zend_op_array *op_array;
            file_handle.filename = const_cast<char*>(full_compile_filename.c_str());
            file_handle.free_filename = 0;
            file_handle.type = ZEND_HANDLE_FILENAME;
            file_handle.opened_path = NULL;
            op_array = zend_compile_file(&file_handle, ZEND_INCLUDE TSRMLS_CC);
            if (!op_array) {
                zend_error(E_ERROR, "Error parsing script:%s", full_compile_filename.c_str());
                return;
            }
            zend_destroy_file_handle(&file_handle TSRMLS_CC);

            SIMPLATE_G(global_string.str(std::string())); // let global_string empty.
            int (*old_output_func)(const char*, unsigned int TSRMLS_DC);
            old_output_func = OG(php_body_write);
            OG(php_body_write) = php_my_output_func;
            zend_execute(op_array TSRMLS_CC);
            OG(php_body_write) = old_output_func;

#ifdef ZEND_ENGINE_2
            destroy_op_array(op_array TSRMLS_CC);
#else
            destroy_op_array(op_array);
#endif // ZEND_ENGINE_2
            efree(op_array);

DEBUG_PRINTF("SIMPLATE_G(global_string) = (%ld)", SIMPLATE_G(global_string).str().length());

            if (mode == SIMPLATE_FETCH) {
                *cache_content = estrndup(SIMPLATE_G(global_string).str().c_str(), SIMPLATE_G(global_string).str().length());
            } else {
                if (SIMPLATE_G(global_string).str().length() > 0) {
                    zend_printf("%s", SIMPLATE_G(global_string).str().c_str());
                }
            }

#ifdef PHP_WIN32
            php_stream *strm = php_stream_open_wrapper(const_cast<char*>(full_cache_filename.c_str()), "wb", ENFORCE_SAFE_MODE | REPORT_ERRORS, NULL);
            if (strm) {
                if (php_stream_supports_lock(strm)) {
                    php_stream_lock(strm, LOCK_EX);
                }
                if (SIMPLATE_G(global_string).str().length() > 0) {
                    php_stream_write_string(strm, const_cast<char*>(SIMPLATE_G(global_string).str().c_str()));
                }
                if (php_stream_supports_lock(strm)) {
                    php_stream_lock(strm, LOCK_UN);
                }
                php_stream_close(strm);
            }
#else
            FILE *fp = VCWD_FOPEN(full_cache_filename.c_str(), "wb");
            if (fp == NULL) {
                // fail to create cache
                zend_error(E_ERROR, "fail to create cache:%s", full_cache_filename.c_str());
                return;
            }
            if (fwrite(SIMPLATE_G(global_string).str().c_str(), 1, SIMPLATE_G(global_string).str().length(), fp) != SIMPLATE_G(global_string).str().length()) {
                // fail to write cache
                zend_error(E_ERROR, "fail to write cache:%s", full_cache_filename.c_str());
                // Notice: don't retun, yet.
            }
            fclose(fp);
#endif // PHP_WIN32

        } else {

DEBUG_PRINTF("%s", "cache file exists.");

            if (compile_check) {

DEBUG_PRINTF("%s", "check cache file is compiled.");

                // check cache filestamp
                time_t now;
                time(&now);
                if (now - cache_stat.st_mtime > cache_lifetime) {
DEBUG_PRINTF("%s", "cache file is not compiled.");
                    goto create_cache;
                }
            }

            string file_contents = _readfile(full_cache_filename.c_str() TSRMLS_CC);
            if (mode == SIMPLATE_FETCH) {
                *cache_content = estrndup(file_contents.c_str(), file_contents.length());
            } else {
                zend_printf("%s", file_contents.c_str());
            }
        }
DEBUG_PRINTF("%s", "end function");

        return;
    }

    // the directory for compiled templates exists?
    struct stat compile_dir_stat;
    if (VCWD_STAT(compile_dir, &compile_dir_stat) != -1) {
        if (!S_ISDIR(compile_dir_stat.st_mode)) {
            zend_error(E_ERROR, "%s is not directory", compile_dir);
            return;
        }
    } else {
        // mkdir recursively
        php_stream_context *context = NULL;
        if (!php_stream_mkdir(compile_dir, 0777, (PHP_STREAM_MKDIR_RECURSIVE | REPORT_ERRORS), context)) {
            zend_error(E_ERROR, "fail to mkdir (%s)", compile_dir);
            return;
        }
    }

    // already compiled
    if (_is_compiled(obj, template_dir.c_str(), compile_dir, resource_name, left_delimiter, right_delimiter TSRMLS_CC)) {

    } else {
compile:
        // compile now
        string compiled_file_content = _readfile(full_template_filename.c_str() TSRMLS_CC);

        // prefilter
        zval *plugins = zend_read_property(Z_OBJCE_P(obj), obj, const_cast<char*>("_plugins"), strlen("_plugins"), 1 TSRMLS_CC);
        if (plugins && Z_TYPE_P(plugins) == IS_ARRAY) {
            zval **prefilter;
            if (zend_hash_find(Z_ARRVAL_P(plugins), "prefilter", sizeof("prefilter"), (void**)&prefilter) == SUCCESS
                && Z_TYPE_PP(prefilter) == IS_ARRAY
            ) {
                zval **elem;
                zend_hash_internal_pointer_reset(Z_ARRVAL_PP(prefilter));
                while (zend_hash_get_current_data(Z_ARRVAL_PP(prefilter), (void**)&elem) == SUCCESS) {

DEBUG_PRINTF("prefilter function = (%s)", Z_STRVAL_PP(elem));

                    zval prefilter_function, prefilter_ret;
                    zval zcontent;
                    zval *prefilter_argv[1];
                    prefilter_argv[0] = &zcontent;
                    SET_ZVAL_STRING(zcontent, compiled_file_content.c_str());

                    INIT_ZVAL(prefilter_function);
                    ZVAL_STRING(&prefilter_function, Z_STRVAL_PP(elem), 1);

                    if (call_user_function(EG(function_table), NULL, &prefilter_function, &prefilter_ret, 1, prefilter_argv TSRMLS_CC) == FAILURE) {
                        zval_dtor(&zcontent);
                        zend_error(E_ERROR, "fail to %s", Z_STRVAL_PP(elem));
                        return;
                    }
                    zval_dtor(&prefilter_function);
                    zval_dtor(&zcontent);
                    compiled_file_content = Z_STRVAL(prefilter_ret);

DEBUG_PRINTF("compiled_file_content = (%s)", compiled_file_content.c_str());

                    zval_dtor(&prefilter_ret);
                    zend_hash_move_forward(Z_ARRVAL_PP(prefilter));
                }
            }
        }

        size_t pos = 0;
        size_t tag_start_pos, tag_end_pos;
        size_t element_start_pos, element_end_pos;
        string item;
        char end_comment_tag[12];
        string new_condition;
        sprintf(end_comment_tag, "*%s", right_delimiter);
        while ((pos = compiled_file_content.find(left_delimiter, pos)) != string::npos) {

            // <{$hoge}>
            // ^         :tag_start_pos
            //   ^       :element_start_pos
            //        ^  :element_end_pos
            //          ^:tag_end_pos
            //   ^---^   :item

            tag_start_pos = pos;
            element_start_pos = tag_start_pos + strlen(left_delimiter);
            element_end_pos = compiled_file_content.find(right_delimiter, tag_start_pos);
            if (element_end_pos == string::npos) {
                zend_error(E_ERROR, "No closed delimiter:`%s' in %s", right_delimiter, full_template_filename.c_str());
                return;
            }
            tag_end_pos = element_end_pos + strlen(right_delimiter);

            item.assign(compiled_file_content, element_start_pos, element_end_pos - element_start_pos);
            item = trim(item.c_str());
            string variable;
            string new_variable;

DEBUG_PRINTF("item = (%s)", item.c_str());

            if (item[0] == '$') {

DEBUG_PRINTF("variable item = (%s)", item.c_str());

cond_var:
                new_condition = "<?php echo ";
                i = 0;

                while (element_start_pos + i < element_end_pos) {
                    if (compiled_file_content[element_start_pos + i] == '+'
                        || (compiled_file_content[element_start_pos + i] == '-'
                            && compiled_file_content.at(element_start_pos + i + 1) != '>')
                        || compiled_file_content[element_start_pos + i] == '*'
                        || compiled_file_content[element_start_pos + i] == '/'
                        || compiled_file_content[element_start_pos + i] == '%'
                        || compiled_file_content[element_start_pos + i] == '('
                        || compiled_file_content[element_start_pos + i] == ')'
                        || compiled_file_content[element_start_pos + i] == ','
                        || compiled_file_content[element_start_pos + i] == '\''
                        || compiled_file_content[element_start_pos + i] == '"'
                        || compiled_file_content[element_start_pos + i] == '?'
                        || compiled_file_content[element_start_pos + i] == ':'
                        || compiled_file_content[element_start_pos + i] == ';'
                        || compiled_file_content[element_start_pos + i] == ','
                        || compiled_file_content[element_start_pos + i] == ' '
                    ) {

                        if (variable != "$") {
                            new_variable = parse_variable(variable.c_str());

DEBUG_PRINTF("(%s) = (%s)", variable.c_str(), new_variable.c_str());

                        } else {
                            new_variable = variable;
                        }
                        new_condition += new_variable;
                        new_condition += compiled_file_content[element_start_pos + i];
                        variable = "";
                    } else {
                        variable += compiled_file_content[element_start_pos + i];
DEBUG_PRINTF("variable = (%s)", variable.c_str());
                    }
                    i++;
                }
                new_variable = parse_variable(variable.c_str());

DEBUG_PRINTF("(%s) = (%s)", variable.c_str(), new_variable.c_str());

                new_condition += new_variable;
                new_condition += "; ?>";

DEBUG_PRINTF("new_condition = (%s)", new_condition.c_str());

                compiled_file_content.replace(tag_start_pos, tag_end_pos - tag_start_pos, new_condition);
                pos += new_condition.length() - 1; // 20060527
            } else if (item[0] == '*') { // Comment
                if (item[item.length() - 1] != '*') { // support multi-line comment
                    tag_end_pos = compiled_file_content.find(end_comment_tag, tag_end_pos) + strlen(end_comment_tag);
                }

DEBUG_PRINTF("COMMENT = (%s)", compiled_file_content.substr(tag_start_pos, tag_end_pos - tag_start_pos).c_str());

                compiled_file_content.erase(tag_start_pos, tag_end_pos - tag_start_pos);
                pos = tag_start_pos - 1;
            } else {

DEBUG_PRINTF("CONDITION = (%s)", item.c_str());

                string command_tag;
                string condition;
                if (item.find(" ") != string::npos) {
                    command_tag = item.substr(0, item.find(" "));
                    condition = item.substr(item.find(" ") + 1);
                } else {
                    command_tag = item;
                }

DEBUG_PRINTF("COMMAND = (%s)", command_tag.c_str());
DEBUG_PRINTF("CONDITION = (%s)", condition.c_str());

                if (command_tag == "if") {
                    new_condition = "<?php if (";
cond_if:
                    i = 0;
                    while (i < condition.length()) {
                        if (condition[i] == '+'
                            || (condition[i] == '-' && condition.at(i + 1) != '>')
                            || condition[i] == '*'
                            || condition[i] == '/'
                            || condition[i] == '%'
                            || condition[i] == '<'
                            || (condition[i] == '>' && condition.at(i - 1) != '-')
                            || condition[i] == '='
                            || condition[i] == '!'
                            || condition[i] == '|'
                            || condition[i] == '&'
                            || condition[i] == '('
                            || condition[i] == ')'
                            || condition[i] == ','
                            || condition[i] == ' '
                        ) {
                            new_variable = parse_variable(variable.c_str());

DEBUG_PRINTF("(%s) = (%s)", variable.c_str(), new_variable.c_str());

                            new_condition += new_variable;
                            new_condition += condition[i];
                            variable = "";
                        } else {
                            variable += condition[i];
                        }
                        i++;
                    }
                    new_variable = parse_variable(variable.c_str());
                    new_condition += new_variable;
                    new_condition += ") { ?>";
                    compiled_file_content.replace(tag_start_pos, tag_end_pos - tag_start_pos, new_condition);
                    pos += new_condition.length() - 1; // 20060527
                } else if (command_tag == "/if") {
                    compiled_file_content.replace(tag_start_pos, tag_end_pos - tag_start_pos, "<?php } ?>");
                    pos += strlen("<?php } ?>") - 1; // 20060527
                } else if (command_tag == "else") {
                    if (item.find("else if") != string::npos) {
                        new_condition = "<?php } else if (";
                        condition = item.substr(item.find("else if") + strlen("else if"));
                        condition = trim(condition.c_str());
                        goto cond_if;
                    }
                    compiled_file_content.replace(tag_start_pos, tag_end_pos - tag_start_pos, "<?php } else { ?>");
                    pos += strlen("<?php } else { ?>") - 1; // 20060527
                } else if (command_tag == "elseif") {
                    new_condition = "<?php } else if (";
                    goto cond_if;
                } else if (command_tag == "section") {
                    string section_name = get_element(item.c_str(), "name");
                    string section_loop = get_element(item.c_str(), "loop");
                    string section_start = get_element(item.c_str(), "start");
                    string section_step = get_element(item.c_str(), "step");
                    string new_section_loop2 = "";
                    condition = section_loop;

                    i = 0;
                    while (i < condition.length()) {
                        if (condition[i] == '+'
                            || condition[i] == '-'
                            || condition[i] == '*'
                            || condition[i] == '/'
                            || condition[i] == '%'
                            || condition[i] == '<'
                            || condition[i] == '>'
                            || condition[i] == '='
                            || condition[i] == '!'
                            || condition[i] == '|'
                            || condition[i] == '&'
                            || condition[i] == '('
                            || condition[i] == ')'
                        ) {
                            new_variable = parse_variable(variable.c_str());
                            new_section_loop2 += new_variable;
                            new_section_loop2 += condition[i];
DEBUG_PRINTF("variable = (%s), new_section_loop2 = (%s)", variable.c_str(), new_section_loop2.c_str());
                            variable = "";
                        } else {
                            if (condition[i] != ' ') {
                                variable += condition[i];
                            }
DEBUG_PRINTF("variable = (%s)", variable.c_str());
                        }
                        i++;
                    }
                    new_section_loop2 += parse_variable(variable.c_str());
DEBUG_PRINTF("new_section_loop2 = (%s)", new_section_loop2.c_str());

                    // "<?php $this->_section['%s']['total'] = count(%s);\n"
                    new_condition = "<?php ";
                    new_condition += "$this->_section['";
                    new_condition += section_name;
                    new_condition += "']['total'] = ";
                    new_condition += "count(";
                    new_condition += new_section_loop2;
                    new_condition += ");\n";

                    // "for ($%s = 0, $this->_section['%s']['iteration'] = 1; $%s<$this->_section['%s']['total']; $%s++, $this->_section['%s']['iteration']++) {\n"
                    new_condition += "for ($";
                    new_condition += section_name;
                    new_condition += " = ";
                    if (section_start.length() > 0) {
                        new_condition += section_start;
                    } else {
                        new_condition += "0";
                    }
                    new_condition += ", $this->_section['";
                    new_condition += section_name;
                    new_condition += "']['iteration'] = 1; $";
                    new_condition += section_name;
                    new_condition += " < $this->_section['";
                    new_condition += section_name;
                    new_condition += "']['total']; $";
                    new_condition += section_name;
                    if (section_step.length() > 0) {
                        new_condition += " += ";
                        new_condition += section_step;
                    } else {
                        new_condition += "++";
                    }
                    new_condition += ", $this->_section['";
                    new_condition += section_name;
                    new_condition += "']['iteration']++";
                    new_condition += ") {\n";

                    // "$this->_section['%s']['first'] = ($% == 0);\n"
                    new_condition += "$this->_section['";
                    new_condition += section_name;
                    new_condition += "']['first'] = ($";
                    new_condition += section_name;
                    new_condition += " == ";
                    if (section_start.length() > 0) {
                        new_condition += section_start;
                    } else {
                        new_condition += "0";
                    }
                    new_condition += ");\n";

                    // "$this->_section['%s']['last'] == ($this->_section['%s']['iteration'] == $this->_section['%s']['total']);\n"
                    new_condition += "$this->_section['";
                    new_condition += section_name;
                    new_condition += "']['last'] = ($this->_section['";
                    new_condition += section_name;
                    new_condition += "']['iteration']";
                    new_condition += " == $this->_section['";
                    new_condition += section_name;
                    new_condition += "']['total']);\n";
                    new_condition += "?>";

                    compiled_file_content.replace(tag_start_pos, tag_end_pos - tag_start_pos, new_condition);
                    pos += new_condition.length() - 1; // 20060527
                } else if (command_tag == "/section") {
                    compiled_file_content.replace(tag_start_pos, tag_end_pos - tag_start_pos, "<?php } ?>");
                    pos += strlen("<?php } ?>") - 1; // 20060527
                } else if (command_tag == "foreach") {
                    string foreach_key = get_element(item.c_str(), "key");
                    string foreach_item = get_element(item.c_str(), "item");
                    string foreach_from = get_element(item.c_str(), "from");

                    string new_foreach_from2 = "";
                    condition = foreach_from;
                    i = 0;
                    while (i < condition.length()) {
                        if (condition[i] == '+'
                            || condition[i] == '-'
                            || condition[i] == '*'
                            || condition[i] == '/'
                            || condition[i] == '%'
                            || condition[i] == '<'
                            || condition[i] == '>'
                            || condition[i] == '='
                            || condition[i] == '!'
                            || condition[i] == '|'
                            || condition[i] == '&'
                            || condition[i] == '('
                            || condition[i] == ')'
                        ) {
                            new_variable = parse_variable(variable.c_str());
                            new_foreach_from2 += new_variable;
                            new_foreach_from2 += condition[i];
                            variable = "";
                        } else {
                            if (condition[i] != ' ') {
                                variable += condition[i];
                            }
                        }
                        i++;
                    }
                    new_foreach_from2 += parse_variable(variable.c_str());

                    // "<?php if (!is_array(%s) && !is_object(%s)) { settype("%s", 'array') };\n"
                    new_condition = "<?php ";
                    new_condition += "if (!is_array(";
                    new_condition += new_foreach_from2;
                    new_condition += ") && !is_object(";
                    new_condition += new_foreach_from2;
                    new_condition += ")) { settype(";
                    new_condition += new_foreach_from2;
                    new_condition += ", 'array'); }\n";

                    // "if (count(%s)) {\n"
                    new_condition += "if (count(";
                    new_condition += new_foreach_from2;
                    new_condition += ")) {\n";

                    // "foreach (%s as %s) { ?>"
                    new_condition += "foreach (";
                    new_condition += new_foreach_from2;
                    new_condition += " as ";
                    if (foreach_key.length() > 0) {
                        new_condition += "$this->_tpl_vars[\"";
                        new_condition += foreach_key;
                        new_condition += "\"] => ";
                    }
                    new_condition += "$this->_tpl_vars[\"";
                    new_condition += foreach_item;
                    new_condition += "\"]) { ?>";

                    compiled_file_content.replace(tag_start_pos, tag_end_pos - tag_start_pos, new_condition);
                    pos += new_condition.length() - 1; // 20060527

                } else if (command_tag == "/foreach") {
                    compiled_file_content.replace(tag_start_pos, tag_end_pos - tag_start_pos, "<?php }} ?>");
                    pos += strlen("<?php }} ?>") - 1; // 20060527
                } else if (command_tag == "include") {
                    string filename = get_element(item.c_str(), "file");
                    string included_filename = get_include_filename(obj, template_dir, filename TSRMLS_CC);
                    new_condition = _readfile(included_filename.c_str() TSRMLS_CC);
                    if (new_condition.find(filename) != string::npos) {
                        zend_error(E_ERROR, "Found recursive include:%s", filename.c_str());
                        return;
                    }
                    compiled_file_content.replace(tag_start_pos, tag_end_pos - tag_start_pos, new_condition);
                    continue; // don't skip with pos
                } else if (command_tag == "php") {
                    string end_php_tag;
                    size_t php_tag_end_pos;
                    end_php_tag  = left_delimiter;
                    end_php_tag += "/php";
                    end_php_tag += right_delimiter;
                    php_tag_end_pos = compiled_file_content.find(end_php_tag, tag_end_pos);
                    new_condition  = "<?php ";
                    new_condition += compiled_file_content.substr(tag_end_pos, php_tag_end_pos - tag_end_pos);
                    new_condition += " ?>";
                    compiled_file_content.replace(tag_start_pos, php_tag_end_pos+end_php_tag.length() - tag_start_pos, new_condition);
                    pos += new_condition.length() - 1;
                } else if (command_tag == "literal") {
                    string end_literal = left_delimiter;
                    end_literal += "/literal";
                    end_literal += right_delimiter;
                    new_condition = compiled_file_content.substr(tag_end_pos, compiled_file_content.find(end_literal, element_end_pos) - tag_end_pos);
                    compiled_file_content.replace(tag_start_pos, compiled_file_content.find(end_literal, element_end_pos) + end_literal.length() - tag_start_pos, new_condition);
                    pos += new_condition.length() - 1;
                } else {
                    goto cond_var;
                }
            }
            pos++;
        }

        // postfilter
        if (plugins && Z_TYPE_P(plugins) == IS_ARRAY) {
            zval **postfilter;
            if (zend_hash_find(Z_ARRVAL_P(plugins), "postfilter", sizeof("postfilter"), (void**)&postfilter) == SUCCESS && Z_TYPE_PP(postfilter) == IS_ARRAY) {
                zval **elem;
                zend_hash_internal_pointer_reset(Z_ARRVAL_PP(postfilter));
                while (zend_hash_get_current_data(Z_ARRVAL_PP(postfilter), (void**)&elem) == SUCCESS) {

                    zval postfilter_function, postfilter_ret;
                    zval zcontent;
                    zval *postfilter_argv[1];
                    postfilter_argv[0] = &zcontent;
                    SET_ZVAL_STRING(zcontent, compiled_file_content.c_str());

                    INIT_ZVAL(postfilter_function);
                    ZVAL_STRING(&postfilter_function, Z_STRVAL_PP(elem), 1);

                    if (call_user_function(EG(function_table), NULL, &postfilter_function, &postfilter_ret, 1, postfilter_argv TSRMLS_CC) == FAILURE) {
                        zval_dtor(&zcontent);
                        zend_error(E_ERROR, "fail to %s", Z_STRVAL_PP(elem));
                        return;
                    }
                    zval_dtor(&postfilter_function);
                    zval_dtor(&zcontent);

                    compiled_file_content = Z_STRVAL(postfilter_ret);
                    zval_dtor(&postfilter_ret);

                    zend_hash_move_forward(Z_ARRVAL_PP(postfilter));
                }
            }
        }

DEBUG_PRINTF("write compiled file path = (%s)", const_cast<char*>(full_compile_filename.c_str()));

#ifdef PHP_WIN32
        php_stream *strm = php_stream_open_wrapper(const_cast<char*>(full_compile_filename.c_str()), "wb", ENFORCE_SAFE_MODE|REPORT_ERRORS, NULL);
        if (!strm) {

            // create sub directory path
            if (full_compile_filename.find(DEFAULT_SLASH) != string::npos) {
                size_t slash_rpos = full_compile_filename.rfind("/");
                size_t default_slash_rpos = full_compile_filename.rfind(DEFAULT_SLASH);

DEBUG_PRINTF("slash_rpos=(%d), default_slash_rpos=(%d)", slash_rpos, default_slash_rpos);

                if (slash_rpos == string::npos || slash_rpos < default_slash_rpos) {
                    slash_rpos = default_slash_rpos;
                }
                string directory_path = full_compile_filename.substr(0, slash_rpos);

DEBUG_PRINTF("slash_rpos=(%d)", slash_rpos);
DEBUG_PRINTF("directory_path=(%s)", const_cast<char*>(directory_path.c_str()));

                // mkdir recursively
                php_stream_context *context = NULL;
                if (!php_stream_mkdir(const_cast<char*>(directory_path.c_str()), 0755, (PHP_STREAM_MKDIR_RECURSIVE | REPORT_ERRORS), context)) {
                    zend_error(E_ERROR, "fail to php_stream_mkdir(win32) (%s)", const_cast<char*>(directory_path.c_str()));
                    return;
                }
                strm = php_stream_open_wrapper(const_cast<char*>(full_compile_filename.c_str()), "wb", ENFORCE_SAFE_MODE|REPORT_ERRORS, NULL);
            }
        }
        if (strm) {
            if (php_stream_supports_lock(strm)) {
                php_stream_lock(strm, LOCK_EX);
            }
            if (compiled_file_content.length() > 0) {
                php_stream_write_string(strm, const_cast<char*>(compiled_file_content.c_str()));
            }
            if (php_stream_supports_lock(strm)) {
                php_stream_lock(strm, LOCK_UN);
            }
            php_stream_close(strm);
        }
#else
        // write compiled file
        FILE *fp = VCWD_FOPEN(full_compile_filename.c_str(), "wb");
        if (fp == NULL) {
            // create sub directory path
            if (full_compile_filename.find(DEFAULT_SLASH) != string::npos) {
                size_t slash_rpos = full_compile_filename.rfind("\\");
                size_t default_slash_rpos = full_compile_filename.rfind(DEFAULT_SLASH);

DEBUG_PRINTF("slash_rpos=(%d), default_slash_rpos=(%d)", slash_rpos, default_slash_rpos);

                if (slash_rpos == string::npos || slash_rpos < default_slash_rpos) {
                    slash_rpos = default_slash_rpos;
                }
                string directory_path = full_compile_filename.substr(0, slash_rpos);

DEBUG_PRINTF("slash_rpos=(%d)", slash_rpos);
DEBUG_PRINTF("directory_path=(%s)", const_cast<char*>(directory_path.c_str()));

                // mkdir recursively
                php_stream_context *context = NULL;
                if (!php_stream_mkdir(const_cast<char*>(directory_path.c_str()), 0755, (PHP_STREAM_MKDIR_RECURSIVE | REPORT_ERRORS), context)) {
                    zend_error(E_ERROR, "fail to php_stream_mkdir (%s)", const_cast<char*>(directory_path.c_str()));
                    return;
                }
            }

            fp = VCWD_FOPEN(full_compile_filename.c_str(), "wb");
            if (fp == NULL) {
                // error handling
                zend_error(E_ERROR, "fail to write : %s", full_compile_filename.c_str());
                return;
            }
        }
        if (fwrite(compiled_file_content.c_str(), 1, compiled_file_content.length(), fp) != compiled_file_content.length()) {
            zend_error(E_WARNING, "fail to write:%s", full_compile_filename.c_str());
            return;
        }
        fclose(fp);
#endif // PHP_WIN32
    }

    *fullfile_name = estrndup(full_compile_filename.c_str(), full_compile_filename.length());

DEBUG_PRINTF("%s", "end function");

    return;
}

/* {{{ Registers simplate properties.
 *
 * @return void
 */
static void register_simplate_properties(TSRMLS_D)
{
    zend_class_entry simplate_ce;

    // register class entry
    INIT_CLASS_ENTRY(simplate_ce, "Simplate", php_simplate_functions);
    simplate_entry_ptr = zend_register_internal_class(&simplate_ce TSRMLS_CC);

#ifdef ZEND_ENGINE_2
    /**
     * Class property
     */
    // directory parameter seting
    zend_declare_property_string(simplate_entry_ptr, const_cast<char*>(TEMPLATE_DIR), strlen(TEMPLATE_DIR), const_cast<char*>(DEFAULT_TEMPLATE_DIR), ZEND_ACC_PUBLIC TSRMLS_CC);
    zend_declare_property_string(simplate_entry_ptr, const_cast<char*>(COMPILE_DIR), strlen(COMPILE_DIR), const_cast<char*>(DEFAULT_COMPILE_DIR), ZEND_ACC_PUBLIC TSRMLS_CC);
    zend_declare_property_string(simplate_entry_ptr, const_cast<char*>(CACHE_DIR), strlen(CACHE_DIR), const_cast<char*>(DEFAULT_CACHE_DIR), ZEND_ACC_PUBLIC TSRMLS_CC);
    zend_declare_property_long(simplate_entry_ptr, const_cast<char*>(CACHING), strlen(CACHING), DEFAULT_CACHING, ZEND_ACC_PUBLIC TSRMLS_CC);
    zend_declare_property_long(simplate_entry_ptr, const_cast<char*>(CACHE_LIFETIME), strlen(CACHE_LIFETIME), DEFAULT_CACHE_LIFETIME, ZEND_ACC_PUBLIC TSRMLS_CC);

    // delimitter
    zend_declare_property_string(simplate_entry_ptr, const_cast<char*>(LEFT_DELIMITER), strlen(LEFT_DELIMITER), const_cast<char*>(DEFAULT_LEFT_DELIMITER), ZEND_ACC_PUBLIC TSRMLS_CC);
    zend_declare_property_string(simplate_entry_ptr, const_cast<char*>(RIGHT_DELIMITER), strlen(RIGHT_DELIMITER), const_cast<char*>(DEFAULT_RIGHT_DELIMITER), ZEND_ACC_PUBLIC TSRMLS_CC);
    zend_declare_property_bool(simplate_entry_ptr, const_cast<char*>(COMPILE_CHECK), strlen(COMPILE_CHECK), DEFAULT_COMPILE_CHECK, ZEND_ACC_PUBLIC TSRMLS_CC);

    // force_compile, lazy_check
    zend_declare_property_bool(simplate_entry_ptr, const_cast<char*>(FORCE_COMPILE), strlen(FORCE_COMPILE), DEFAULT_FORCE_COMPILE, ZEND_ACC_PUBLIC TSRMLS_CC);
    zend_declare_property_bool(simplate_entry_ptr, const_cast<char*>(LAZY_CHECK), strlen(LAZY_CHECK), DEFAULT_LAZY_CHECK, ZEND_ACC_PUBLIC TSRMLS_CC);

    // version
    zend_declare_property_string(simplate_entry_ptr, const_cast<char*>("version"), strlen("version"), const_cast<char*>(VERSION), ZEND_ACC_PUBLIC TSRMLS_CC);

    // temporary
    zend_declare_property_null(simplate_entry_ptr, const_cast<char*>("_tpl_vars"), strlen("_tpl_vars"), ZEND_ACC_PUBLIC TSRMLS_CC);
    // plugins
    zend_declare_property_null(simplate_entry_ptr, const_cast<char*>("_plugins"), strlen("_plugins"), ZEND_ACC_PUBLIC TSRMLS_CC);
#endif // ZEND_ENGINE_2
}
/* }}} */

/* {{{ PHP_MINIT_FUNCTION
 *
 * @return int returns SUCCESS if function succeeded
 */
PHP_MINIT_FUNCTION(simplate)
{
    /* If you have INI entries, uncomment these lines 
    ZEND_INIT_MODULE_GLOBALS(simplate, php_simplate_init_globals, NULL);
    REGISTER_INI_ENTRIES();
    */
    ZEND_INIT_MODULE_GLOBALS(simplate, php_simplate_init_globals, NULL);
    register_simplate_properties(TSRMLS_C);

    return SUCCESS;
}
/* }}} */

/* {{{ PHP_MSHUTDOWN_FUNCTION
 *
 * @return int returns SUCCESS if function succeeded
 */
PHP_MSHUTDOWN_FUNCTION(simplate)
{
    /* uncomment this line if you have INI entries
    UNREGISTER_INI_ENTRIES();
    */
    return SUCCESS;
}
/* }}} */

/* Remove if there's nothing to do at request start */
/* {{{ PHP_RINIT_FUNCTION
 *
 * @return int returns SUCCESS if function succeeded
 */
PHP_RINIT_FUNCTION(simplate)
{
    return SUCCESS;
}
/* }}} */

/* Remove if there's nothing to do at request end */
/* {{{ PHP_RSHUTDOWN_FUNCTION
 *
 * @return int returns SUCCESS if function succeeded
 */
PHP_RSHUTDOWN_FUNCTION(simplate)
{
    return SUCCESS;
}
/* }}} */

/* {{{ PHP_MINFO_FUNCTION
 *
 * @return void
 */
PHP_MINFO_FUNCTION(simplate)
{
    php_info_print_table_start();
    php_info_print_table_header(2, "simplate support", "enabled");
    php_info_print_table_row(2, "Version", VERSION);
    php_info_print_table_end();

    /* Remove comments if you have entries in php.ini
    DISPLAY_INI_ENTRIES();
    */
}
/* }}} */

#ifdef ZEND_ENGINE_2
/* {{{ ZEND_METHOD __construct
 *
 * @return void
 */
ZEND_METHOD(simplate, __construct)
#else
/* {{{ PHP_FUNCTION simplate_init
 *
 * @return void
 */
PHP_FUNCTION(simplate_init)
#endif // ZEND_ENGINE_2
{
#ifdef ZEND_ENGINE_2
#else
  zval *obj = getThis();
  object_init_ex(obj, simplate_entry_ptr);
  add_property_string(obj, TEMPLATE_DIR, DEFAULT_TEMPLATE_DIR, 1);
  add_property_string(obj, COMPILE_DIR, DEFAULT_COMPILE_DIR, 1);
  add_property_string(obj, CACHE_DIR, DEFAULT_CACHE_DIR, 1);
  add_property_long(obj, CACHING, DEFAULT_CACHING);
  add_property_long(obj, CACHE_LIFETIME, DEFAULT_CACHE_LIFETIME);

  add_property_string(obj, LEFT_DELIMITER, DEFAULT_LEFT_DELIMITER, 1);
  add_property_string(obj, RIGHT_DELIMITER, DEFAULT_RIGHT_DELIMITER, 1);
  add_property_bool(obj, COMPILE_CHECK, DEFAULT_COMPILE_CHECK);
  add_property_bool(obj, FORCE_COMPILE, DEFAULT_FORCE_COMPILE);
  add_property_bool(obj, LAZY_CHECK, DEFAULT_LAZY_CHECK);
  add_property_string(obj, "version", VERSION, 1);
#endif // ZEND_ENGINE_2
}
/* }}} */

#ifdef ZEND_ENGINE_2
#else
/* {{{ Set already exist array on new array recursively(PHP4).
 *
 * @param zval **struc     old array
 * @param zval **new_array new array to be set
 *
 * @return void
 */
static void php_set_r(
    zval **struc,
    zval **new_array
)
{
    zval **elem;
    char *k = 0;
    ulong i = 0;
    char buf[128];

    zend_hash_internal_pointer_reset(Z_ARRVAL_PP(struc));
    while (zend_hash_get_current_data(Z_ARRVAL_PP(struc), (void **)&elem) == SUCCESS) {
        zend_hash_get_current_key(Z_ARRVAL_PP(struc), &k, &i, 0);
        sprintf(buf, "%d", i);
        switch (Z_TYPE_PP(elem)) {
        case IS_NULL:
            add_assoc_unset(*new_array, k);
            break;
        case IS_BOOL:
            add_assoc_bool(*new_array, k, Z_BVAL_PP(elem));
            break;
        case IS_LONG:
            add_assoc_long(*new_array, k, Z_LVAL_PP(elem));
            break;
        case IS_DOUBLE:
            add_assoc_double(*new_array, k, Z_DVAL_PP(elem));
            break;
        case IS_STRING:
            add_assoc_stringl(*new_array, k, Z_STRVAL_PP(elem), Z_STRLEN_P(elem)/* (*elem)->value.str.val */, 1);
            break;
        case IS_ARRAY:
            zval_add_ref(elem);
            add_assoc_zval(*new_array, k, *elem);
        }
        zend_hash_move_forward(Z_ARRVAL_PP(struc));
    }
}
#endif // ZEND_ENGINE_2
/* }}} */

#ifdef ZEND_ENGINE_2
/* {{{ ZEND_METHOD assign
 *
 * @return void
 */
ZEND_METHOD(simplate, assign)
#else
/* {{{ PHP_FUNCTION simplate_assign
 *
 * @return void
 */
PHP_FUNCTION(simplate_assign)
#endif // ZEND_ENGINE_2
{
    char *key = NULL;
    zval *zvalue;
    int key_len;

DEBUG_PRINTF("%s", "start function");

    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, const_cast<char*>("sz"), &key, &key_len, &zvalue) == FAILURE) {
        return;
    }
    zval *obj = getThis();
    zval *new_array;

#ifdef ZEND_ENGINE_2
    new_array = zend_read_property(Z_OBJCE_P(obj), obj, const_cast<char*>("_tpl_vars"), strlen("_tpl_vars"), 1 TSRMLS_CC);
    // initialize.use new_array on the first time.
    if (new_array == EG(uninitialized_zval_ptr) || Z_TYPE_P(new_array) == IS_NULL) {
        MAKE_STD_ZVAL(new_array);
        array_init(new_array);
        Z_DELREF_P(new_array);
    }
#else
    zval **tmp;
    MAKE_STD_ZVAL(new_array);
    array_init(new_array);
    // the ``_tpl_vars'' exists already.
    if (zend_hash_find(Z_OBJPROP_P(obj), "_tpl_vars", sizeof("_tpl_vars"), (void**)&tmp) == SUCCESS) {
        // set old array to new array recursively.
        php_set_r(tmp, &new_array);
    }
#endif // ZEND_ENGINE_2

    switch (Z_TYPE_P(zvalue)) {
    case IS_NULL:
        add_assoc_unset(new_array, key);
        break;
    case IS_BOOL:
        add_assoc_bool(new_array, key, Z_BVAL_P(zvalue));
        break;
    case IS_DOUBLE:
        add_assoc_double(new_array, key, Z_DVAL_P(zvalue));
        break;
    case IS_STRING:
        add_assoc_stringl(new_array, key, Z_STRVAL_P(zvalue), Z_STRLEN_P(zvalue), 1);
        break;
    case IS_LONG:
        add_assoc_long(new_array, key, Z_LVAL_P(zvalue));
        break;
    case IS_ARRAY:
        zval_add_ref(&zvalue); // don't forget this!
        add_assoc_zval(new_array, key, zvalue);
        break;
    case IS_OBJECT:
        zval_add_ref(&zvalue);
        add_assoc_zval(new_array, key, zvalue);
    }

#ifdef ZEND_ENGINE_2
    zend_update_property(Z_OBJCE_P(obj), obj, const_cast<char*>("_tpl_vars"), strlen("_tpl_vars"), new_array TSRMLS_CC);
#else
    zend_hash_update(Z_OBJPROP_P(obj), "_tpl_vars", strlen("_tpl_vars") + 1, &new_array, sizeof(zval*), NULL);
#endif // ZEND_ENGINE_2

DEBUG_PRINTF("%s", "end function");

    return;
}
/* }}} */

/* {{{ Outputs to global_string.
 *
 * @param const char *str
 * @param uint       strlen
 *
 * @return int returns strlen
 */
static int php_my_output_func(
    const char *str,
    uint str_len TSRMLS_DC
)
{
    SIMPLATE_G(global_string) << str;
    return str_len;
}
/* }}} */

#ifdef ZEND_ENGINE_2
/* {{{ ZEND_METHOD fetch
 *
 * @return void
 */
ZEND_METHOD(simplate, fetch)
#else
/* {{{ PHP_FUNCTION simplate_fetch
 *
 * @return void
 */
PHP_FUNCTION(simplate_fetch)
#endif // ZEND_ENGINE_2
{
    string error;
    char *fullfile_name = NULL;
    char *fetch_content = NULL;

DEBUG_PRINTF("%s", "start function");

    read_parse_template(INTERNAL_FUNCTION_PARAM_PASSTHRU, &fullfile_name, SIMPLATE_FETCH, &fetch_content);
    if (fetch_content) {
        RETURN_STRING(fetch_content, 1);
    }

    if (!fullfile_name || strlen(fullfile_name) <= 0) {
        return;
    }

    //
    // Execute compiled file
    //
    zend_file_handle file_handle;
    zend_op_array *op_array;
    file_handle.filename = fullfile_name;
    file_handle.free_filename = 0;
    file_handle.type = ZEND_HANDLE_FILENAME;
    file_handle.opened_path = NULL;
    op_array = zend_compile_file(&file_handle, ZEND_INCLUDE TSRMLS_CC);
    if (!op_array) {
        zend_error(E_ERROR, "Error parsing script:%s", fullfile_name);
        return;
    }
    zend_destroy_file_handle(&file_handle TSRMLS_CC);

#ifdef USE_ZEND_EXECUTE
    SIMPLATE_G(global_string.str(std::string()));
    int (*old_output_func)(const char*, unsigned int TSRMLS_DC);
    old_output_func = OG(php_body_write);
    OG(php_body_write) = php_my_output_func;
    zend_execute(op_array TSRMLS_CC);
    OG(php_body_write) = old_output_func;
    RETURN_STRING(const_cast<char*>(SIMPLATE_G(global_string).str().c_str()), 1);
#else
    // ob_start();
    zval *output_handler = NULL;
    zend_bool erase = 1;
    long chunk_size = 0;
    if (php_start_ob_buffer(output_handler, chunk_size, erase TSRMLS_CC) == FAILURE) {
        zend_error(E_ERROR, "Error: fail to ob_start");
        RETURN_FALSE;
    }
    // include
    string include_execute = "include '";
    include_execute += fullfile_name;
    include_execute += "';";
    zend_eval_string(const_cast<char*>(include_execute.c_str()), NULL, const_cast<char*>("simplate") TSRMLS_CC);

#ifdef ZEND_ENGINE_2
    destroy_op_array(op_array TSRMLS_CC);
#else
    destroy_op_array(op_array);
#endif // ZEND_ENGINE_2
    efree(op_array);
    efree(fullfile_name);
    if (php_ob_get_buffer(return_value TSRMLS_CC) == FAILURE) {
        RETURN_FALSE;
    }
    php_end_ob_buffer(0, 0 TSRMLS_CC);
#endif // USE_ZEND_EXECUTE

DEBUG_PRINTF("%s", "end function");
}
/* }}} */

#ifdef ZEND_ENGINE_2
/* {{{ ZEND_METHOD display
 *
 * @return void
 */
ZEND_METHOD(simplate, display)
#else
/* {{{ PHP_FUNCTION simplate_display
 *
 * @return void
 */
PHP_FUNCTION(simplate_display)
#endif // ZEND_ENGINE_2
{
DEBUG_PRINTF("%s", "start function");

    char *fullfile_name = NULL;
    read_parse_template(INTERNAL_FUNCTION_PARAM_PASSTHRU, &fullfile_name, SIMPLATE_DISPLAY);
    if (!fullfile_name || strlen(fullfile_name) <= 0) {
DEBUG_PRINTF("%s", "end function");
        return;
    }

DEBUG_PRINTF("full file name = (%s)", fullfile_name);

    //
    // Execute compiled file
    //
#ifdef USE_ZEND_EXECUTE
    zend_file_handle file_handle;
    zend_op_array *op_array;
    file_handle.filename = full_compile_filename;
    file_handle.free_filename = 0;
    file_handle.type = ZEND_HANDLE_FILENAME;
    file_handle.opened_path = NULL;
    op_array = zend_compile_file(&file_handle, ZEND_INCLUDE TSRMLS_CC);
    if (!op_array) {
        zend_error(E_ERROR, "Error parsing script:%s", full_compile_filename.c_str());
        efree(fullfile_name);
        return;
    }
    zend_destroy_file_handle(&file_handle TSRMLS_CC);
    zend_execute(op_array TSRMLS_CC);
#else
    // This way is safer
    string include_execute = "include '";
    include_execute += fullfile_name;
    include_execute += "';";

DEBUG_PRINTF("include_execute = (%s)", include_execute.c_str());

    zend_eval_string(const_cast<char*>(include_execute.c_str()), NULL, const_cast<char*>("simplate") TSRMLS_CC);
#endif // USE_ZEND_EXECUTE
    efree(fullfile_name);

DEBUG_PRINTF("%s", "end function");

    return;
}
/* }}} */

#ifdef ZEND_ENGINE_2
/* {{{ ZEND_METHOD clear_cache
 *
 * @return void
 */
ZEND_METHOD(simplate, clear_cache)
#else
/* {{{ PHP_FUNCTION simplate_clear_cache
 *
 * @return void
 */
PHP_FUNCTION(simplate_clear_cache)
#endif // ZEND_ENGINE_2
{
    zval *obj;
    char *resource_name = NULL;
    int resource_name_len = 0;
    long caching = 0;
    char *cache_dir;
    string error;

    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, const_cast<char*>("s"), &resource_name, &resource_name_len) == FAILURE) {
        RETURN_FALSE;
    }

    obj = getThis();
#ifdef ZEND_ENGINE_2
    caching = Z_LVAL_P(zend_read_property(Z_OBJCE_P(obj), obj, const_cast<char*>(CACHING), strlen(CACHING), 1 TSRMLS_CC));
#else
#endif // ZEND_ENGINE_2
    if (caching) {
#ifdef ZEND_ENGINE_2
        cache_dir = Z_STRVAL_P(zend_read_property(Z_OBJCE_P(obj), obj, const_cast<char*>(CACHE_DIR), strlen(CACHE_DIR), 1 TSRMLS_CC));
#else
#endif // ZEND_ENGINE_2

        if (cache_dir[strlen(cache_dir) - 1] == DEFAULT_SLASH) {
            cache_dir[strlen(cache_dir) - 1] = '\0';
        }

        struct stat cache_stat;
        // cache directory exists?
        if (VCWD_STAT(cache_dir, &cache_stat) != -1) {
            if (!S_ISDIR(cache_stat.st_mode)) {
                zend_error(E_ERROR, "does not exist cache directory:%s", cache_dir);
            }
        }
        string full_cache_filename(cache_dir);
        full_cache_filename += DEFAULT_SLASH;
        full_cache_filename += resource_name;
        if (VCWD_STAT(full_cache_filename.c_str(), &cache_stat) != -1) {
            unlink(full_cache_filename.c_str());
        }
    }
}
/* }}} */

/* {{{ Registers filter into plugins.
 *
 * @param INTERNAL_FUNCTION_PARAMETERS see zend.h
 * @param char *filter_name filter's name
 *
 * @return void
 */
void register_plugins(INTERNAL_FUNCTION_PARAMETERS, char *filter_name)
{
    char *function_name = NULL;
    int function_name_len = 0;

DEBUG_PRINTF("%s", "start function");

    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, const_cast<char*>("s"), &function_name, &function_name_len) == FAILURE) {
        return;
    }

DEBUG_PRINTF("filter_name = (%s)", filter_name);
DEBUG_PRINTF("function_name = (%s)", function_name);

    zval *obj = getThis();
    zval *_plugins;

#ifdef ZEND_ENGINE_2
    _plugins = zend_read_property(Z_OBJCE_P(obj), obj, const_cast<char*>("_plugins"), strlen("_plugins"), 1 TSRMLS_CC);
    // initialize.use _plugins on the first time.
    if (_plugins == EG(uninitialized_zval_ptr) || Z_TYPE_P(_plugins) == IS_NULL) {
        MAKE_STD_ZVAL(_plugins);
        array_init(_plugins);
        Z_DELREF_P(_plugins);
    }
#else
#endif // ZEND_ENGINE_2
//php_var_dump(&_plugins, 1 TSRMLS_CC);

    zval **zfilter;
    // filter
    if (zend_hash_find(Z_ARRVAL_P(_plugins), filter_name, strlen(filter_name) + 1, (void**)&zfilter) == SUCCESS) {

//php_var_dump(zfilter, 1 TSRMLS_CC);
        add_assoc_string(*zfilter, function_name, function_name, 1);

    } else {
        zval *data;
        MAKE_STD_ZVAL(data);
        array_init(data);

        add_assoc_string(data, function_name, function_name, 1);
        add_assoc_zval(_plugins, filter_name, data);
    }

#ifdef ZEND_ENGINE_2
    zend_update_property(Z_OBJCE_P(obj), obj, const_cast<char*>("_plugins"), strlen("_plugins"), _plugins TSRMLS_CC);
#else
#endif // ZEND_ENGINE_2

DEBUG_PRINTF("%s", "end function");
}
/* }}} */

#ifdef ZEND_ENGINE_2
/* {{{ ZEND_METHOD register_prefilter
 *
 * @return void
 */
ZEND_METHOD(simplate, register_prefilter)
#else
/* {{{ PHP_FUNCTION simplate_register_prefilter
 *
 * @return void
 */
PHP_FUNCTION(simplate_register_prefilter)
#endif // ZEND_ENGINE_2
{
    register_plugins(INTERNAL_FUNCTION_PARAM_PASSTHRU, const_cast<char*>("prefilter"));
}
/* }}} */

#ifdef ZEND_ENGINE_2
/* {{{ ZEND_METHOD register_postfilter
 *
 * @return void
 */
ZEND_METHOD(simplate, register_postfilter)
#else
/* {{{ PHP_FUNCTION simplate_register_postfilter
 *
 * @return void
 */
PHP_FUNCTION(simplate_register_postfilter)
#endif // ZEND_ENGINE_2
{
    register_plugins(INTERNAL_FUNCTION_PARAM_PASSTHRU, const_cast<char*>("postfilter"));
}
/* }}} */

/*
 * Local variables:
 * tab-width: 4
 * c-basic-offset: 4
 * indent-tabs-mode: nil
 * End:
 * vim600: noet sw=4 ts=4 sts=4 fdm=marker
 * vim<600: noet sw=4 ts=4 sts=4
 */
