/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   RadixTree.hpp                                      :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: vdurand <vdurand@student.42lyon.fr>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/04/23 01:21:20 by vdurand           #+#    #+#             */
/*   Updated: 2026/04/23 01:39:03 by vdurand          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef _RADIXTREE_H
# define _RADIXTREE_H

# include <string>
# include <vector>
# include <utility>
# include <stdexcept>
# include "Utils/Optional.hpp"

template<typename T>
class RadixTree
{
	struct Node
	{
		std::string			label;
		Optional<T>			value;
		std::vector<Node*>	children;

		Node() : label("") {}
		explicit Node(const std::string& str) : label(str) {}

		~Node()
		{
			for (size_t i = 0; i < children.size(); ++i)
				delete children[i];
		}

	private:
		Node(const Node&);
		Node&	operator=(const Node&);
	};

	struct Frame
	{
		Node*		node;
		std::string	prefix;
		size_t		child_idx;

		Frame(Node* n, const std::string& p)
			: node(n), prefix(p), child_idx(0) {}
	};

	typedef std::vector<Frame> Stack;

public:
	typedef std::string						key_type;
	typedef T								mapped_type;
	typedef std::pair<const std::string, T>	value_type;
	typedef size_t							size_type;
	typedef ptrdiff_t						difference_type;

	class iterator
	{
		friend class RadixTree;

	public:
		typedef std::forward_iterator_tag		iterator_category;
		typedef std::pair<const std::string, T>	value_type;
		typedef ptrdiff_t						difference_type;
		typedef value_type						*pointer;
		typedef value_type&						reference;

		iterator() : tree(NULL) {}

		reference	operator*()  { return *current; }
		pointer		operator->() { return  current.operator->(); }

		iterator&	operator++()
		{
			this->advance();
			return *this;
		}

		iterator	operator++(int)
		{
			iterator tmp(*this);
			this->advance();
			return tmp;
		}

		bool	operator==(const iterator& o) const
		{
			if (tree == NULL && o.tree == NULL)
				return true;
			if (tree != o.tree)
				return false;
			if (stack.empty() && o.stack.empty())
				return true;
			if (stack.empty() || o.stack.empty())
				return false;
			return stack.back().node == o.stack.back().node;
		}

		bool	operator!=(const iterator& o) const { return !(*this == o); }

	private:
		RadixTree*				tree;
		Stack					stack;
		Optional<value_type>	current;

		iterator(RadixTree* t, Node* root) : tree(t)
		{
			if (root && (!root->children.empty() || root->value.has_value()))
			{
				stack.push_back(Frame(root, ""));
				this->settle();
			}
		}

		explicit	iterator(RadixTree* t) : tree(t) {}

		void	settle()
		{
			while (!stack.empty())
			{
				Frame&		top = stack.back();
				Node		*n = top.node;
				std::string	full_key = top.prefix + n->label;

				while (top.child_idx < n->children.size())
				{
					Node* child = n->children[top.child_idx++];
					stack.push_back(Frame(child, full_key));
					stack.pop_back();
					break;
				}

				if (n->value.has_value())
				{
					current = Optional<value_type>(value_type(full_key, n->value.value()));
					return ;
				}

				this->advance_frame();
			}
			current = Optional<value_type>();
		}

		void	advance()
		{
			if (stack.empty()) return;
			this->advance_frame();
			this->settle();
		}

		void	advance_frame()
		{
			while (!stack.empty())
			{
				Frame& top = stack.back();
				if (top.child_idx < top.node->children.size())
				{
					Node*  child	 = top.node->children[top.child_idx++];
					std::string pref = top.prefix + top.node->label;
					stack.push_back(Frame(child, pref));
					return;
				}
				stack.pop_back();
			}
		}
	};

	class const_iterator
	{
		friend class RadixTree;

	public:
		typedef std::forward_iterator_tag		iterator_category;
		typedef std::pair<const std::string, T>	value_type;
		typedef ptrdiff_t						difference_type;
		typedef const value_type*				pointer;
		typedef const value_type&				reference;

		const_iterator() {}
		const_iterator(const iterator& it) : it(it) {}

		reference	operator*()  const { return *it; }
		pointer		operator->() const { return  it.operator->(); }

		const_iterator&	operator++()	{ ++it; return *this; }
		const_iterator	operator++(int) { const_iterator t(*this); ++it; return t; }

		bool	operator==(const const_iterator& o) const { return it == o.it; }
		bool	operator!=(const const_iterator& o) const { return it != o.it; }

	private:
		iterator	it;
		explicit	const_iterator(RadixTree* t, Node* root) : it(t, root) {}
		explicit	const_iterator(RadixTree* t) : it(t) {}
	};

	RadixTree() : root(""), size(0) {}
	~RadixTree() {}

private:
	RadixTree(const RadixTree&);
	RadixTree&	operator=(const RadixTree&);

public:
	iterator		begin()			{ return iterator(this, &root); }
	iterator		end()			{ return iterator(this); }
	const_iterator	begin() const	{ return const_iterator(const_cast<RadixTree*>(this), const_cast<Node*>(&root)); }
	const_iterator	end() const		{ return const_iterator(const_cast<RadixTree*>(this)); }
	const_iterator	cbegin() const	{ return begin(); }
	const_iterator	cend() const		{ return end(); }

	bool	  empty() const { return size == 0; }
	size_type size()  const { return size; }

	std::pair<iterator, bool>	insert(const value_type& kv)
	{
		const std::string& key = kv.first;
		Node* target = this->ins(&root, key, 0, kv.second, true);
		bool  inserted = target != NULL;
		iterator it = this->find_iter(key);
		return std::make_pair(it, inserted);
	}

	T&	operator[](const std::string& key)
	{
		this->ins(&root, key, 0, T(), true);
		Node* n = this->find_node(&root, key, 0);
		return n->value.value();
	}

	bool	erase(const std::string& key)
	{
		return this->_erase(&root, key, 0);
	}

	void	clear()
	{
		for (size_t i = 0; i < root.children.size(); ++i)
			delete root.children[i];
		this->root.children.clear();
		this->root.value = Optional<T>();
		this->size = 0;
	}

	void	swap(RadixTree& other)
	{
		this->root.children.swap(other.root.children);
		std::swap(this->root.value, other.root.value);
		std::swap(this->size, other.size);
	}

	iterator	find(const std::string& key)
	{
		Node *n = find_node(&this->root, key, 0);
		if (n == NULL)
			return end();
		return find_iter(key);
	}

	const_iterator	find(const std::string& key) const
	{
		const Node *n = find_node_count(&this->root, key, 0);
		if (n == NULL)
			return end();
		return const_cast<RadixTree*>(this)->find_iter(key);
	}

	iterator	find_prefix(const std::string& key)
	{
		std::string	found_key;
		Node		*n = find_prefix_node(&this->root, key, 0, found_key);
		if (n == NULL)
			return end();
		return find_iter(found_key);
	}

	size_type	count(const std::string& key) const
	{
		return (this->find_node_count(&root, key, 0) != NULL) ? 1 : 0;
	}

	bool	search(const std::string& key, T& val) const
	{
		const Node *n = this->find_node_count(&this->root, key, 0);
		if (n == NULL)
			return false;
		val = n->value.value();
		return true;
	}

	bool	search_prefix(const std::string& key, T& val) const
	{
		std::string	dummy;
		const Node	*n = this->find_prefix_node_c(&this->root, key, 0, dummy);
		if (n == NULL)
			return false;
		val = n->value.value();
		return true;
	}

	void	insert(const std::string& key, const T& val)
	{
		insert(value_type(key, val));
	}

private:

	Node	  root;
	size_type size;

	Node	*ins(Node *n, const std::string& key, size_t i, const T& val, bool only_if_absent)
	{
		if (i == key.size())
		{
			if (only_if_absent && n->value.has_value())
				return NULL;
			if (!n->value.has_value())
				this->size++;
			n->value = Optional<T>(val);
			return n;
		}

		for (size_t c = 0; c < n->children.size(); ++c)
		{
			Node	*ch = n->children[c];
			size_t	k = this->common_prefix(ch->label, key, i);

			if (!k)
				continue;

			if (k == ch->label.size())
				return this->ins(ch, key, i + k, val, only_if_absent);

			Node *mid = new Node(ch->label.substr(0, k));
			ch->label = ch->label.substr(k);
			mid->children.push_back(ch);
			n->children[c] = mid;

			if (i + k < key.size())
			{
				Node* leaf = new Node(key.substr(i + k));
				mid->children.push_back(leaf);
				return this->ins(leaf, key, key.size(), val, only_if_absent);
			}
			else
			{
				return this->ins(mid, key, key.size(), val, only_if_absent);
			}
		}
		Node *leaf = new Node(key.substr(i));
		n->children.push_back(leaf);
		return this->ins(leaf, key, key.size(), val, only_if_absent);
	}

	Node	*find_node(Node *n, const std::string& key, size_t i)
	{
		if (i == key.size())
			return n->value.has_value() ? n : NULL;

		for (size_t c = 0; c < n->children.size(); ++c)
		{
			Node	*ch = n->children[c];
			size_t	k  = this->common_prefix(ch->label, key, i);
			if (!k)
				continue;
			if (k == ch->label.size())
				return this->find_node(ch, key, i + k);
		}
		return NULL;
	}

	const Node	*find_node_count(const Node* n, const std::string& key, size_t i) const
	{
		if (i == key.size())
			return n->value.has_value() ? n : NULL;

		for (size_t c = 0; c < n->children.size(); ++c)
		{
			const Node	*ch = n->children[c];
			size_t		k = this->common_prefix(ch->label, key, i);
			if (!k)
				continue;
			if (k == ch->label.size())
				return this->find_node_count(ch, key, i + k);
		}
		return NULL;
	}

	Node	*find_prefix_node(Node* n, const std::string& key, size_t i, std::string& found_key)
	{
		if (n->value.has_value())
		{
			found_key = key.substr(0, i);
			return n;
		}
		if (i == key.size())
			return NULL;

		for (size_t c = 0; c < n->children.size(); ++c)
		{
			Node*  ch = n->children[c];
			size_t k  = common_prefix(ch->label, key, i);
			if (!k) continue;
			if (k == ch->label.size())
			{
				Node* res = find_prefix_node(ch, key, i + k, found_key);
				if (res) return res;
			}
		}
		return NULL;
	}

	const Node	*find_prefix_node_c(const Node* n, const std::string& key, size_t i, std::string& found_key) const
	{
		if (n->value.has_value())
		{
			found_key = key.substr(0, i);
			return n;
		}
		if (i == key.size())
			return NULL;

		for (size_t c = 0; c < n->children.size(); ++c)
		{
			const Node* ch = n->children[c];
			size_t k = common_prefix(ch->label, key, i);
			if (!k) continue;
			if (k == ch->label.size())
			{
				const Node* res = find_prefix_node_c(ch, key, i + k, found_key);
				if (res) return res;
			}
		}
		return NULL;
	}

	bool	_erase(Node *n, const std::string& key, size_t i)
	{
		if (i == key.size())
		{
			if (!n->value.has_value())
				return false;
			n->value = Optional<T>();
			this->size--;
			return true;
		}

		for (size_t c = 0; c < n->children.size(); ++c)
		{
			Node	*ch = n->children[c];
			size_t	k = this->common_prefix(ch->label, key, i);
			if (!k)
				continue;
			if (k == ch->label.size())
			{
				bool removed = this->_erase(ch, key, i + k);
				if (removed)
				{
					if (!ch->value.has_value() && ch->children.size() == 1)
					{
						Node* grandchild = ch->children[0];
						grandchild->label = ch->label + grandchild->label;
						ch->children.clear();
						delete ch;
						n->children[c] = grandchild;
					}
					else if (!ch->value.has_value() && ch->children.empty())
					{
						delete ch;
						n->children.erase(n->children.begin() + c);
					}
				}
				return removed;
			}
		}
		return false;
	}

	iterator	find_iter(const std::string& key)
	{
		iterator	it(this);
		Node		*n = &root;
		size_t		i = 0;

		it.stack.push_back(Frame(n, ""));

		while (i < key.size())
		{
			bool found = false;
			for (size_t c = 0; c < n->children.size(); ++c)
			{
				Node	*ch = n->children[c];
				size_t	k = common_prefix(ch->label, key, i);
				if (!k)
					continue;
				if (k == ch->label.size())
				{
					it.stack.back().child_idx = c + 1;
					it.stack.push_back(Frame(ch, key.substr(0, i)));
					n = ch;
					i += k;
					found = true;
					break;
				}
			}
			if (!found)
				return end();
		}

		Node* target = it.stack.back().node;
		if (!target->value.has_value())
			return end();
		it.current = Optional<value_type>( value_type(key, target->value.value()));
		return it;
	}

	static size_t	common_prefix(const std::string& label const std::string& key, size_t i)
	{
		size_t k = 0;
		while (k < label.size() && i + k < key.size() && label[k] == key[i + k])
			++k;
		return k;
	}
};

template<typename T>
void	swap(RadixTree<T>& a, RadixTree<T>& b)
{
	a.swap(b);
}

#endif // _RADIXTREE_H
