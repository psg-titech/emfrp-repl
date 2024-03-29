/** -------------------------------------------
 * @file   ast.c
 * @brief  Emfrp AST implementation
 * @author Go Suzuki <puyogo.suzuki@gmail.com>
 * @date   2023/3/21
 ------------------------------------------- */

#include "ast.h"
#include "string_t.h"
#include <stdio.h>

const char * const binary_op_table[] = {"+",  "-", "/",  "*",  "%", "<<", ">>", "<=", "<",
                                        ">=", ">", "==", "!=", "&", "|",  "^",  "&&", "||"};

void
deconstructor_free_shallow(deconstructor_t * st)
{
  switch(st->kind) {
    case DECONSTRUCTOR_IDENTIFIER:
      break;
    case DECONSTRUCTOR_TUPLE:
      if(st->value.tuple.tag != nullptr) {  // ILLEGAL STATE
        DEBUGBREAK;
        string_free(st->value.tuple.tag);
        em_free(st->value.tuple.tag);
      }
      for(list_t * li = st->value.tuple.data; li != nullptr;) {
        deconstructor_free_shallow((deconstructor_t *)(&(li->value)));
        list_t * ne = LIST_NEXT(li);
        free(li);
        li = ne;
      }
      break;
    default:
      break;
  }
}

void
deconstructor_free_deep(deconstructor_t * st)
{
  switch(st->kind) {
    case DECONSTRUCTOR_IDENTIFIER:
      string_free(st->value.identifier);
      em_free(st->value.identifier);
      break;
    case DECONSTRUCTOR_TUPLE:
      if(st->value.tuple.tag != nullptr) {
        string_free(st->value.tuple.tag);
        em_free(st->value.tuple.tag);
      }
      for(list_t * li = st->value.tuple.data; li != nullptr;) {
        deconstructor_free_deep((deconstructor_t *)(&(li->value)));
        list_t * ne = LIST_NEXT(li);
        free(li);
        li = ne;
      }
      break;
    default:
      break;
  }
}

void
parser_toplevel_free_shallow(parser_toplevel_t * pt)
{
  switch(pt->kind) {
    case PARSER_TOPLEVEL_KIND_NODE:
      parser_node_free_shallow(pt->value.node);
      break;
    case PARSER_TOPLEVEL_KIND_DATA:
      parser_data_free_deep(pt->value.data);
      break;
    case PARSER_TOPLEVEL_KIND_FUNC:
      parser_func_free_shallow(pt->value.func);
      break;
    case PARSER_TOPLEVEL_KIND_EXPR:
      parser_expression_free(pt->value.expression);
      break;
    case PARSER_TOPLEVEL_KIND_RECORD:
      parser_record_free_deep(pt->value.record);
      break;
  }
  em_free(pt);
}

void
parser_toplevel_free_deep(parser_toplevel_t * pt)
{
  switch(pt->kind) {
    case PARSER_TOPLEVEL_KIND_NODE:
      parser_node_free_deep(pt->value.node);
      break;
    case PARSER_TOPLEVEL_KIND_DATA:
      parser_data_free_deep(pt->value.data);
      break;
    case PARSER_TOPLEVEL_KIND_FUNC:
      parser_func_free_deep(pt->value.func);
      break;
    case PARSER_TOPLEVEL_KIND_EXPR:
      parser_expression_free(pt->value.expression);
      break;
    case PARSER_TOPLEVEL_KIND_RECORD:
      parser_record_free_deep(pt->value.record);
      break;
  }
  em_free(pt);
}

void
parser_func_free_shallow(parser_func_t * pf)
{
  string_free(pf->name);
  em_free(pf);
}

void
parser_func_free_deep(parser_func_t * pf)
{
  string_free(pf->name);
  em_free(pf->name);
  if(pf->arguments != nullptr) {
    deconstructor_t dt = {
      DECONSTRUCTOR_TUPLE, .value.tuple.tag = nullptr, .value.tuple.data = pf->arguments};
    deconstructor_free_deep(&dt);
  }
  parser_expression_free(pf->expression);
  em_free(pf);
}

void
parser_data_free_deep(parser_data_t * pd)
{
  deconstructor_free_deep(&(pd->name));
  parser_expression_free(pd->expression);
  em_free(pd);
}

void
parser_record_free_deep(parser_record_t * pr)
{
  string_free(&(pr->name));
  list_t /*<string_t>*/ * li = pr->accessors;
  while(li != nullptr) {
    list_t *   ne = LIST_NEXT(li);
    string_t * v  = (string_t *)(&(li->value));
    string_free(v);
    em_free(li);
    li = ne;
  }
  em_free(pr);
}

void
parser_node_free_shallow(parser_node_t * pn)
{
  deconstructor_free_shallow(&(pn->name));
  parser_expression_free(pn->init_expression);
  em_free(pn);
}

void
parser_node_free_deep(parser_node_t * pn)
{
  deconstructor_free_deep(&(pn->name));
  if(pn->as != nullptr) {
    string_free(pn->as);
    em_free(pn->as);
  }
  parser_expression_free(pn->expression);
  if(pn->init_expression != nullptr) parser_expression_free(pn->init_expression);
  em_free(pn);
}

void go_deconstructor_list_print(list_t /*<deconstructor_t>*/ * li);

void
go_deconstructor_print(deconstructor_t * dt)
{
  switch(dt->kind) {
    case DECONSTRUCTOR_ANY:
      fputs("_", stdout);
      break;
    case DECONSTRUCTOR_IDENTIFIER:
      fputs(dt->value.identifier->buffer, stdout);
      break;
    case DECONSTRUCTOR_TUPLE:
      if(dt->value.tuple.tag != nullptr) {
        fputs(dt->value.tuple.tag->buffer, stdout);
        fputs(" ", stdout);
      }
      go_deconstructor_list_print(dt->value.tuple.data);
      break;
    case DECONSTRUCTOR_INTEGER:
      printf("%d", dt->value.integer);
      break;
#if EMFRP_ENABLE_FLOATING
    case DECONSTRUCTOR_FLOATING:
      printf("%lf", dt->value.floating);
      break;
#endif
    default:
      DEBUGBREAK;
      break;
  }
}

void
go_deconstructor_list_print(list_t /*<deconstructor_t>*/ * li)
{
  if(li == nullptr) return;
  fputs("(", stdout);
  go_deconstructor_print((deconstructor_t *)(&(li->value)));
  for(li = LIST_NEXT(li); li != nullptr; li = LIST_NEXT(li)) {
    fputs(", ", stdout);
    go_deconstructor_print((deconstructor_t *)(&(li->value)));
  }
  fputs(")", stdout);
}

void
parser_toplevel_print(parser_toplevel_t * t)
{
  switch(t->kind) {
    case PARSER_TOPLEVEL_KIND_NODE:
      parser_node_print(t->value.node);
      break;
    case PARSER_TOPLEVEL_KIND_DATA:
      parser_data_print(t->value.data);
      break;
    case PARSER_TOPLEVEL_KIND_FUNC:
      parser_function_print(t->value.func);
      break;
    case PARSER_TOPLEVEL_KIND_EXPR:
      parser_expression_print(t->value.expression);
      break;
    case PARSER_TOPLEVEL_KIND_RECORD:
      parser_record_print(t->value.record);
      break;
    default:
      DEBUGBREAK;
      break;
  }
}

void
parser_node_print(parser_node_t * n)
{
  fputs("node ", stdout);
  go_deconstructor_print(&(n->name));
  if(n->init_expression != nullptr) {
    fputs(" init[", stdout);
    parser_expression_print(n->init_expression);
    fputs("]", stdout);
  }
  if(n->as != nullptr) {
    fputs(" as ", stdout);
    fputs(n->as->buffer, stdout);
  }
  fputs(" = ", stdout);
  parser_expression_print(n->expression);
  fputs("\n", stdout);
}

void
parser_data_print(parser_data_t * d)
{
  fputs("data ", stdout);
  go_deconstructor_print(&(d->name));
  fputs(" = ", stdout);
  parser_expression_print(d->expression);
}

void
parser_function_print(parser_func_t * f)
{
  fputs("func ", stdout);
  fputs(f->name->buffer, stdout);
  go_deconstructor_list_print(f->arguments);
  fputs(" = ", stdout);
  parser_expression_print(f->expression);
}

void
parser_record_print(parser_record_t * r)
{
  fputs("record ", stdout);
  fputs(r->name.buffer, stdout);
  fputs("(", stdout);
  for(list_t /*<string_t>*/ * li = r->accessors; li != nullptr; li = LIST_NEXT(li)) {
    string_t * v = (string_t *)(&(li->value));
    fputs(v->buffer, stdout);
    if(LIST_NEXT(li) != nullptr) fputs(", ", stdout);
  }
  fputs(")", stdout);
}

void
parser_expression_print(parser_expression_t * e)
{
  if(EXPR_KIND_IS_INTEGER(e)) printf("%d", ((int)(size_t)e) >> 2);
#if EMFRP_ENABLE_FLOATING
  else if(EXPR_KIND_IS_FLOAT(e))
    printf("%f", uninline_float(e));
#endif
  else if(EXPR_KIND_IS_BOOLEAN(e))
    fputs(EXPR_IS_TRUE(e) ? "True" : "False", stdout);
  else if(e == nullptr)
    fputs("NIL", stdout);
  else {
    switch(e->kind) {
      case EXPR_KIND_FLOATING:
        printf("%f", e->value.floating);
        break;
      case EXPR_KIND_IDENTIFIER:
        printf("%s", e->value.identifier.buffer);
        break;
      case EXPR_KIND_LAST_IDENTIFIER:
        printf("%s@last", e->value.identifier.buffer);
        break;
      case EXPR_KIND_IF:
        fputs("if ", stdout);
        parser_expression_print(e->value.ifthenelse.cond);
        fputs(" then ", stdout);
        parser_expression_print(e->value.ifthenelse.then);
        fputs(" else ", stdout);
        parser_expression_print(e->value.ifthenelse.otherwise);
        break;
      case EXPR_KIND_CASE:
        parser_expression_print(e->value.caseof.of);
        fputs(": ", stdout);
        {
          parser_branch_list_t * bl = e->value.caseof.branches;
          if(bl == nullptr) break;
          go_deconstructor_print(bl->deconstruct);
          fputs(" -> ", stdout);
          parser_expression_print(bl->body);
          bl = bl->next;
          for(; bl != nullptr; bl = bl->next) {
            fputs(", ", stdout);
            go_deconstructor_print(bl->deconstruct);
            fputs(" -> ", stdout);
            parser_expression_print(bl->body);
          }
          break;
        }
      case EXPR_KIND_BEGIN:
        printf(" { ");
        {
          parser_branch_list_t * bl = e->value.begin.branches;
          if(bl == nullptr) break;
          if(bl->deconstruct != nullptr) {
            go_deconstructor_print(bl->deconstruct);
            printf(" <- ");
          }
          parser_expression_print(bl->body);
          bl = bl->next;
          for(; bl != nullptr; bl = bl->next) {
            fputs("; ", stdout);
            if(bl->deconstruct != nullptr) {
              go_deconstructor_print(bl->deconstruct);
              fputs(" <- ", stdout);
            }
            parser_expression_print(bl->body);
          }
          fputs(" } ", stdout);
          break;
        }
      case EXPR_KIND_TUPLE:
        fputs("(", stdout);
        for(parser_expression_tuple_list_t * tl = &(e->value.tuple); tl != nullptr; tl = tl->next) {
          parser_expression_print(tl->value);
          if(tl->next != nullptr) fputs(", ", stdout);
        }
        fputs(")", stdout);
        break;
      case EXPR_KIND_FUNCCALL:
        fputs("(", stdout);
        parser_expression_print(e->value.funccall.callee);
        fputs(")", stdout);
        {
          fputs("(", stdout);
          parser_expression_tuple_list_t * tl = &(e->value.funccall.arguments);
          if(tl->value != nullptr)
            for(; tl != nullptr; tl = tl->next) {
              parser_expression_print(tl->value);
              if(tl->next != nullptr) fputs(", ", stdout);
            }
          fputs(")", stdout);
        }
        break;
      case EXPR_KIND_FUNCTION: {
        fputs("(fun", stdout);
        go_deconstructor_list_print(e->value.function.arguments);
        fputs(" -> (", stdout);
        parser_expression_print(e->value.function.body);
        fputs("))", stdout);
        break;
      }
      default:
        if(EXPR_KIND_IS_BIN_OP(e)) {
          fputs("(", stdout);
          parser_expression_print(e->value.binary.lhs);
          fputs(" ", stdout);
          fputs(binary_op_table[e->kind >> PARSER_EXPRESSION_KIND_SHIFT], stdout);
          fputs(" ", stdout);
          parser_expression_print(e->value.binary.rhs);
          fputs(")", stdout);
          break;
        } else
          DEBUGBREAK;
    }
  }
}

void
parser_expression_free(parser_expression_t * expr)
{
  if(!EXPR_IS_POINTER(expr)) return;  // It is not pointer.
  if(expr == nullptr) return;
  switch(expr->kind) {
    case EXPR_KIND_IDENTIFIER:
    case EXPR_KIND_LAST_IDENTIFIER:
      string_free(&(expr->value.identifier));
      break;
    case EXPR_KIND_IF:
      parser_expression_free(expr->value.ifthenelse.cond);
      parser_expression_free(expr->value.ifthenelse.then);
      parser_expression_free(expr->value.ifthenelse.otherwise);
      break;
    case EXPR_KIND_CASE:
      parser_expression_free(expr->value.caseof.of);
      for(parser_branch_list_t * bl = expr->value.caseof.branches; bl != nullptr;) {
        deconstructor_free_deep(bl->deconstruct);
        parser_expression_free(bl->body);
        parser_branch_list_t * prev = bl;
        bl                          = bl->next;
        em_free(prev);
      }
      break;
    case EXPR_KIND_BEGIN:
      for(parser_branch_list_t * bl = expr->value.begin.branches; bl != nullptr;) {
        if(bl->deconstruct != nullptr) deconstructor_free_deep(bl->deconstruct);
        parser_expression_free(bl->body);
        parser_branch_list_t * prev = bl;
        bl                          = bl->next;
        em_free(prev);
      }
      break;
    case EXPR_KIND_TUPLE: {
      parser_expression_tuple_list_t * tl = &expr->value.tuple;
      parser_expression_free(tl->value);
      tl = tl->next;
      while(tl != nullptr) {
        parser_expression_tuple_list_t * v = tl->next;
        parser_expression_free(tl->value);
        free(tl);
        tl = v;
      }
      break;
    }
    case EXPR_KIND_FUNCCALL: {
      parser_expression_tuple_list_t * tl = &expr->value.funccall.arguments;
      tl                                  = tl->next;
      while(tl != nullptr) {
        parser_expression_tuple_list_t * v = tl->next;
        parser_expression_free(tl->value);
        free(tl);
        tl = v;
      }
      parser_expression_free(expr->value.funccall.callee);
      break;
    }
    case EXPR_KIND_FUNCTION: {
      deconstructor_t dt = {
        DECONSTRUCTOR_TUPLE, .value.tuple.tag = nullptr,
        .value.tuple.data = expr->value.function.arguments};
      expr->value.function.reference_count--;
      if(expr->value.function.reference_count > 0) return;
      parser_expression_free(expr->value.function.body);

      deconstructor_free_deep(&dt);
      em_free(expr->value.function.arguments);
      break;
    }
    default:
      if(EXPR_KIND_IS_BIN_OP(expr)) {
        parser_expression_free(expr->value.binary.lhs);
        parser_expression_free(expr->value.binary.rhs);
      } else
        DEBUGBREAK;
  }
  free(expr);
}
