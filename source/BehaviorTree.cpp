#include <iostream>
#include "BehaviorTree.hpp"

explicit SelectorNode::SelectorNode(std::vector<BTNodePtr> children) : m_children(children) {}

explicit SequenceNode::SequenceNode(std::vector<BTNodePtr> children) : m_children(children) {}

explicit ConditionNode::ConditionNode(std::string name)				 : m_name(name) {}

explicit ActionNode::ActionNode(std::string command)				 : m_command(command) {}

explicit BehaviorTree::BehaviorTree(BTNodePtr root)	                 : m_root(std::move(root)){}			



[[nodiscard]] BTResult SelectorNode::tick(const BTContext& context) const  {

}

[[nodiscard]] BTResult SequenceNode::tick(const BTContext& context) const  {

}

[[nodiscard]] BTResult ConditionNode::tick(const BTContext& context) const  {

}

[[nodiscard]] BTResult ActionNode::tick(const BTContext& context) const  {

}


[[nodiscard]] std::string BehaviorTree::tick(const BTContext& context) const{

}
