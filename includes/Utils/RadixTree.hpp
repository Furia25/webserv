/* ************************************************************************** */
/*																			*/
/*														:::	  ::::::::   */
/*   RadixTree.hpp									  :+:	  :+:	:+:   */
/*													+:+ +:+		 +:+	 */
/*   By: vdurand <vdurand@student.42lyon.fr>		+#+  +:+	   +#+		*/
/*												+#+#+#+#+#+   +#+		   */
/*   Created: 2026/04/16 17:38:46 by vdurand		   #+#	#+#			 */
/*   Updated: 2026/04/16 17:44:38 by vdurand		  ###   ########.fr	   */
/*																			*/
/* ************************************************************************** */

#ifndef _RADIXTREE_H
# define _RADIXTREE_H
# include <string>
# include <vector>
# include <map>

template<typename T>
class RadixTree
{
public:
	RadixTree() : root("") {}

	void insert(const std::string& key, const T& val) { _ins(&root, key, 0, val); };

	bool search(const std::string& key, T& val) const { return _srch(&root, key, 0, val); };

	bool search_prefix(const std::string& key, T& val) const { return _srch_prefix(&root, key, 0, val); };

private:
private:
	struct Node
	{
		std::string			label;
		bool				has_value;
		T					value;
		std::vector<Node*>	children;

		Node(const std::string& l) : label(l), has_value(false), value() {}
		~Node()
		{
			for (size_t i = 0; i < children.size(); ++i)
				delete children[i];
		}
	};

	Node root;

	void _ins(Node* n, const std::string& key, size_t i, const T& val);

	bool _srch(const Node* n, const std::string& key, size_t i, T& val) const;

	bool _srch_prefix(const Node* n, const std::string& key, size_t i, T& val) const;
};

template <typename T>
inline void RadixTree<T>::_ins(Node *n, const std::string &key, size_t i, const T &val)
{
	if (i == key.size())
	{
		n->has_value = true;
		n->value = val;
		return;
	}

	for (size_t c = 0; c < n->children.size(); ++c)
	{
		Node* ch = n->children[c];
		size_t k = 0;
		while (k < ch->label.size() && i + k < key.size() && ch->label[k] == key[i + k])
			++k;
		if (!k)
			continue;

		if (k == ch->label.size() && k == key.size() - i)
			_ins(ch, key, i + k, val);
		else if (k == ch->label.size())
			_ins(ch, key, i + k, val);
		else
		{
			Node* mid = new Node(ch->label.substr(0, k));
			ch->label = ch->label.substr(k);
			mid->children.push_back(ch);
			n->children[c] = mid;

			if (i + k < key.size())
			{
				Node* leaf = new Node(key.substr(i + k));
				mid->children.push_back(leaf);
				_ins(leaf, key, key.size(), val);
			} else
				_ins(mid, key, key.size(), val);
		}
		return;
	}
	Node* leaf = new Node(key.substr(i));
	n->children.push_back(leaf);
	_ins(leaf, key, key.size(), val);
}

template <typename T>
inline bool RadixTree<T>::_srch(const Node *n, const std::string &key, size_t i, T &val) const
{
	if (i == key.size())
	{
		if (n->has_value)
		{
			val = n->value;
			return true;
		}
		return false;
	}
	for (size_t c = 0; c < n->children.size(); ++c)
	{
		const Node* ch = n->children[c];
		size_t k = 0;
		while (k < ch->label.size() && i + k < key.size() && ch->label[k] == key[i + k])
			++k;
		if (!k)
			continue;
		if (k == ch->label.size() && _srch(ch, key, i + k, val))
			return true;
	}
	return false;
}

template <typename T>
inline bool RadixTree<T>::_srch_prefix(const Node *n, const std::string &key, size_t i, T &val) const
{
	if (n->has_value)
	{
		val = n->value;
		return true;
	}
	if (i == key.size())
		return false;

	for (size_t c = 0; c < n->children.size(); ++c)
	{
		const Node* ch = n->children[c];

		size_t k = 0;
		while (k < ch->label.size() && i + k < key.size() && ch->label[k] == key[i + k])
			++k;

		if (!k)
			continue;

		if (k == ch->label.size())
		{
			if (_srch_prefix(ch, key, i + k, val))
				return true;
		}
		else if (i + k == key.size())
			return false;
	}
	return false;
}

#endif // _RADIXTREE_H
