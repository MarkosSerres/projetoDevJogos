#include "BehaviorTree.hpp"
#include <assert.h>

SelectorNode::SelectorNode(std::vector<BTNodePtr> children) : m_children(std::move(children)) { 
    for (const auto& child : m_children)
        assert(child != nullptr);
}

SequenceNode::SequenceNode(std::vector<BTNodePtr> children) : m_children(std::move(children)) {
    for(const auto& child : m_children)
        assert(child != nullptr);
}

ConditionNode::ConditionNode(std::string name) : m_name(std::move(name)) {}

ActionNode::ActionNode(std::string command)    : m_command(std::move(command)) {}

BehaviorTree::BehaviorTree(BTNodePtr root)     : m_root(std::move(root)){}			


BTResult SelectorNode::tick(const BTContext& context) const {
    for(const BTNodePtr& child : m_children){
        BTResult result = child->tick(context);
        if(result.success)
            return result;
    }
    return {false, ""};
}

BTResult SequenceNode::tick(const BTContext& context) const  {
    std::string lastCommand;

    for(const BTNodePtr& child : m_children){
        BTResult result = child->tick(context);
        if(!result.success) 
            return {false, ""}; 

        if(!result.command.empty())
            lastCommand = std::move(result.command);
    }
    return {true, std::move(lastCommand)};
}

BTResult ConditionNode::tick(const BTContext& context) const  {
    for(const auto& fact : context){
        if(fact.first == m_name && fact.second != 0) 
            return {true, ""};
    }
    return {false, ""};
}

BTResult ActionNode::tick(const BTContext& context) const  {
    return {true, m_command};
}

std::string BehaviorTree::tick(const BTContext& context) const{
    if(!m_root) return "";

    BTResult result = m_root->tick(context);

    return (result.success) ? std::move(result.command) : "";
}