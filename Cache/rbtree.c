#include <stdio.h>
#include <stdlib.h>
#include "cache.h"
#include "op.h"
#include "rbtree.h"

void _left_rotate(_rb_tree_ptr T, _rb_tree_node_ptr _x)
{
	_rb_tree_node_ptr _y = _x->_right;

	_x->_right = _y->_left;

	if (_y->_left != NULL)
		_y->_left->_parent = _x;

	_y->_parent = _x->_parent;

	if (_x->_parent == NULL)
		T->_root = _y;
	else if (_x->_parent->_left == _x)
		_x->_parent->_left = _y;
	else
		_x->_parent->_right = _y;

	_y->_left = _x;
	_x->_parent = _y;
}

void _right_rotate(_rb_tree_ptr T, _rb_tree_node_ptr _x)
{
	_rb_tree_node_ptr _y = _x->_left;

	_x->_left = _y->_right;

	if (_y->_right != NULL)
		_y->_right->_parent = _x;

	_y->_parent = _x->_parent;

	if (_x->_parent == NULL)
		T->_root = _y;
	else if (_x->_parent->_right == _x)
		_x->_parent->_right = _y;
	else
		_x->_parent->_left = _y;

	_y->_right = _x;
	_x->_parent = _y;
}

void _rb_tree_insert_fix(_rb_tree_ptr T, _rb_tree_node_ptr _x)
{
	while (_x != T->_root && _x->_parent->_color == _red)
	{
		if (_x->_parent == _x->_parent->_parent->_left)
		{
			_rb_tree_node_ptr _y = _x->_parent->_parent->_right;
			if (_y != NULL && _y->_color == _red)
			{
				_x->_parent->_color = _black;
				_y->_color = _black;
				_x->_parent->_parent->_color = _red;
				_x = _x->_parent->_parent;
			}
			else
			{
				if (_x == _x->_parent->_right)
				{
					_x = _x->_parent;
					_left_rotate(T, _x);
				}
				_x->_parent->_color = _black;
				_x->_parent->_parent->_color = _red;
				_right_rotate(T, _x->_parent->_parent);
			}
		}
		else
		{
			_rb_tree_node_ptr _y = _x->_parent->_parent->_left;
			if (_y != NULL && _y->_color == _red)
			{
				_x->_parent->_color = _black;
				_y->_color = _black;
				_x->_parent->_parent->_color = _red;
				_x = _x->_parent->_parent;
			}
			else
			{
				if (_x == _x->_parent->_left)
				{
					_x = _x->_parent;
					_right_rotate(T, _x);
				}
				_x->_parent->_color = _black;
				_x->_parent->_parent->_color = _red;
				_left_rotate(T, _x->_parent->_parent);
			}
		}
	}
	T->_root->_color = _black;
}

int _rb_tree_insert(_rb_tree_ptr T, uint64_t key, uint64_t value)
{
	_rb_tree_node_ptr _y = NULL;
	_rb_tree_node_ptr _x = T->_root;

	_rb_tree_node_ptr _k = (_rb_tree_node_ptr)malloc(sizeof(_rb_tree_node));
	_k->_color = _red;
	_k->_left = NULL;
	_k->_right = NULL;
	_k->_parent = NULL;
	_k->key = key;
	_k->value = value;

	while (_x != NULL)
	{
		_y = _x;
		if (key < _x->key)
			_x = _x->_left;
		else if (key > _x->key)
			_x = _x->_right;
		else
			return -1;
	}
	_k->_parent = _y;
	if (_y == NULL)
		T->_root = _k;
	else
	{
		if (key < _y->key)
			_y->_left = _k;
		else
			_y->_right = _k;
	}


	_rb_tree_insert_fix(T, _k);
	return 0;
}

_rb_tree_node_ptr _rb_tree_find(_rb_tree_ptr T, uint64_t key)
{
	_rb_tree_node_ptr _y = NULL;
	_rb_tree_node_ptr _x = T->_root;
	while (_x != NULL)
	{
		if (key < _x->key)
			_x = _x->_left;
		else if (key > _x->key)
			_x = _x->_right;
		else
		{
			_y = _x;
			break;
		}
	}
	return _y;
}

void _rb_tree_node_clear(_rb_tree_node_ptr T)
{
	if (T == NULL)
		return;
	_rb_tree_clear(T->_left);
	_rb_tree_clear(T->_right);
	free(T);
}

void _rb_tree_clear(_rb_tree_ptr T)
{
	_rb_tree_node_clear(T->_root);
	free(T);
}