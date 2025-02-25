/* This entire file is licensed under GNU General Public License v3.0
 *
 * Copyright 2022- sfwbar maintainers
 */

#include <glib.h>
#include "../config.h"
#include "../scanner.h"
#include "../sway_ipc.h"
#include "../sfwbar.h"
#include "../client.h"

static gboolean config_var_flag ( GScanner *scanner, gint *flag )
{
  if(((gint)g_scanner_peek_next_token(scanner) >= G_TOKEN_SUM) &&
    ((gint)(scanner->next_token) <= G_TOKEN_FIRST))
    *flag = g_scanner_get_next_token(scanner);
  return TRUE;
}

static gboolean config_var_type (GScanner *scanner, gint *type )
{
  gint token = g_scanner_get_next_token(scanner);
  if(token == G_TOKEN_REGEX || token == G_TOKEN_JSON || token == G_TOKEN_GRAB)
    *type = token;
  else
    g_scanner_error(scanner,"invalid parser");
  return !scanner->max_parse_errors;
}

static void config_var ( GScanner *scanner, ScanFile *file )
{
  gchar *vname = NULL, *pattern = NULL;
  guint type;
  gint flag = G_TOKEN_LASTW;

  config_parse_sequence(scanner,
      SEQ_REQ,G_TOKEN_IDENTIFIER,NULL,&vname,NULL,
      SEQ_REQ,'=',NULL,NULL,"Missing '=' in variable declaration",
      SEQ_REQ,-2,(parse_func)config_var_type,&type,NULL,
      SEQ_REQ,'(',NULL,NULL,"Missing '(' after parser",
      SEQ_END);

  if(scanner->max_parse_errors)
    return g_free(vname);

  switch(type)
  {
    case G_TOKEN_REGEX:
    case G_TOKEN_JSON:
      config_parse_sequence(scanner,
          SEQ_REQ,G_TOKEN_STRING,NULL,&pattern,"Missing pattern in parser",
          SEQ_OPT,',',NULL,NULL,NULL,
          SEQ_CON,-2,(parse_func)config_var_flag,&flag,NULL,
          SEQ_REQ,')',NULL,NULL,"Missing ')' after parser",
          SEQ_OPT,';',NULL,NULL,NULL,
          SEQ_END);
      break;
    case G_TOKEN_GRAB:
      config_parse_sequence(scanner,
          SEQ_OPT,-2,(parse_func)config_var_flag,&flag,NULL,
          SEQ_REQ,')',NULL,NULL,"Missing ')' after parser",
          SEQ_OPT,';',NULL,NULL,NULL,
          SEQ_END);
      break;
    default:
      g_scanner_error(scanner,"invalid parser for variable %s",vname);
  }

  if(scanner->max_parse_errors)
  {
    g_free(vname);
    g_free(pattern);
    return;
  }

  scanner_var_new(vname,file,pattern,type,flag);
  g_free(vname);
  g_free(pattern);
}

static gboolean config_source_flags ( GScanner *scanner, gint *flags )
{
  while ( g_scanner_peek_next_token(scanner) == ',' )
  {
    g_scanner_get_next_token(scanner);
    g_scanner_get_next_token(scanner);
    if((gint)scanner->token == G_TOKEN_NOGLOB)
      *flags |= VF_NOGLOB;
    else if((gint)scanner->token == G_TOKEN_CHTIME)
      *flags |= VF_CHTIME;
    else
        g_scanner_error(scanner, "invalid flag in source");
  }
  return !scanner->max_parse_errors;
}

static ScanFile *config_source ( GScanner *scanner, gint source )
{
  ScanFile *file;
  gchar *fname = NULL, *trigger = NULL;
  gint flags = 0;

  switch(source)
  {
    case SO_FILE:
      config_parse_sequence(scanner,
          SEQ_REQ,'(',NULL,NULL,"Missing '(' after source",
          SEQ_REQ,G_TOKEN_STRING,NULL,&fname,"Missing file in a source",
          SEQ_OPT,-2,(parse_func)config_source_flags,&flags,NULL,
          SEQ_REQ,')',NULL,NULL,"Missing ')' after source",
          SEQ_REQ,'{',NULL,NULL,"Missing '{' after source",
          SEQ_END);
      break;
    case SO_CLIENT:
      config_parse_sequence(scanner,
          SEQ_REQ,'(',NULL,NULL,"Missing '(' after source",
          SEQ_REQ,G_TOKEN_STRING,NULL,&fname,"Missing file in a source",
          SEQ_OPT,',',NULL,NULL,NULL,
          SEQ_CON,G_TOKEN_STRING,NULL,&trigger,NULL,
          SEQ_REQ,')',NULL,NULL,"Missing ')' after source",
          SEQ_REQ,'{',NULL,NULL,"Missing '{' after source",
          SEQ_END);
      break;
    default:
      config_parse_sequence(scanner,
          SEQ_REQ,'(',NULL,NULL,"Missing '(' after source",
          SEQ_REQ,G_TOKEN_STRING,NULL,&fname,"Missing file in a source",
          SEQ_REQ,')',NULL,NULL,"Missing ')' after source",
          SEQ_REQ,'{',NULL,NULL,"Missing '{' after source",
          SEQ_END);
      break;
  }

  if(scanner->max_parse_errors)
  {
    g_free(fname);
    g_free(trigger);
    return NULL;
  }

  file = scanner_file_new ( source, fname, trigger, flags );

  while(g_scanner_peek_next_token(scanner) == G_TOKEN_IDENTIFIER)
    config_var(scanner, file);

  config_parse_sequence(scanner,
      SEQ_REQ,'}',NULL,NULL,"Expecting a variable declaration or '}'",
      SEQ_OPT,';',NULL,NULL,NULL,
      SEQ_END);

  return file;
}

void config_scanner ( GScanner *scanner )
{
  ScanFile *file;
  scanner->max_parse_errors = FALSE;

  if(!config_expect_token(scanner, '{', "Missing '{' after 'scanner'"))
    return;
  g_scanner_get_next_token(scanner);

  while(((gint)g_scanner_peek_next_token(scanner) != '}' )&&
      ( (gint)g_scanner_peek_next_token ( scanner ) != G_TOKEN_EOF ))
  {
    switch((gint)g_scanner_get_next_token(scanner))
    {
      case G_TOKEN_FILE:
        config_source(scanner,SO_FILE);
        break;
      case G_TOKEN_EXEC:
        config_source(scanner,SO_EXEC);
        break;
      case G_TOKEN_MPDCLIENT:
        file = config_source(scanner,SO_CLIENT);
        client_mpd(file);
        break;
      case G_TOKEN_SWAYCLIENT:
        file = config_source(scanner,SO_CLIENT);
        sway_ipc_client_init(file);
        break;
      case G_TOKEN_EXECCLIENT:
        file = config_source(scanner,SO_CLIENT);
        client_exec(file);
        break;
      case G_TOKEN_SOCKETCLIENT:
        file = config_source(scanner,SO_CLIENT);
        client_socket(file);
        break;
      default:
        g_scanner_error(scanner, "Unexpected declaration in scanner");
        break;
    }
  }
  if((gint)scanner->next_token == '}')
    g_scanner_get_next_token(scanner);
}
