#pragma once
#ifndef REBTREE_H_
#define REBTREE_H_

typedef char _color_type;
#define _red 0
#define _black 1

typedef struct _rb_tree_node* _rb_tree_node_ptr;
typedef struct _rb_tree_node
{
	_color_type _color;
	_rb_tree_node_ptr _parent;
	_rb_tree_node_ptr _left;
	_rb_tree_node_ptr _right;
	uint64_t key;
	uint64_t value;
}_rb_tree_node;

typedef struct _rb_tree* _rb_tree_ptr;
typedef struct _rb_tree
{
	_rb_tree_node_ptr _root;
}_rb_tree;

void _left_rotate(_rb_tree_ptr T, _rb_tree_node_ptr _x);

void _right_rotate(_rb_tree_ptr T, _rb_tree_node_ptr _x);

void _rb_tree_insert_fix(_rb_tree_ptr T, _rb_tree_node_ptr _x);

int _rb_tree_insert(_rb_tree_ptr T, uint64_t key, uint64_t value);

_rb_tree_node_ptr _rb_tree_find(_rb_tree_ptr T, uint64_t key);

void _rb_tree_node_clear(_rb_tree_node_ptr T);

void _rb_tree_clear(_rb_tree_ptr T);

#endif