#include <iostream>
#include "BehaviorTree.hpp"

namespace {
    BTResult makeFailure() {
        return BTResult{false, std::string{}};
    }
}

explicit SelectorNode::SelectorNode(std::vector<BTNodePtr> children) : m_children(std::move(children)) {}

explicit SequenceNode::SequenceNode(std::vector<BTNodePtr> children) : m_children(std::move(children)) {}

explicit ConditionNode::ConditionNode(std::string name)				 : m_name(std::move(name)) {}

explicit ActionNode::ActionNode(std::string command)				 : m_command(std::move(command)) {}

explicit BehaviorTree::BehaviorTree(BTNodePtr root)	                 : m_root(std::move(root)){}			



[[nodiscard]] BTResult SelectorNode::tick(const BTContext& context) const {
    for(const BTNodePtr& child : m_children){
        if(!child)
            continue; 

        BTResult result = child->tick(context);
        if(result.success)
            return result;
    }
    return makeFailure();
}

[[nodiscard]] BTResult SequenceNode::tick(const BTContext& context) const  {
    std::string lastCommand;

    for(const BTNodePtr& child : m_children){
        if(!child)
            return makeFailure();

        BTResult result = child->tick(context);
        if(!result.success) 
            return makeFailure(); 

        if(!result.command.empty())
            lastCommand = std::move(result.command);
    }
    return BTResult{true, std::move(lastCommand)};
}

[[nodiscard]] BTResult ConditionNode::tick(const BTContext& context) const  {
    for(const auto& fact : context){
        if(fact.first == m_name && fact.second != 0) 
            return BTResult{true, std::string{}};
    }
    return makeFailure();
}

[[nodiscard]] BTResult ActionNode::tick(const BTContext& context) const  {
    return BTResult{true, m_command};
}


[[nodiscard]] std::string BehaviorTree::tick(const BTContext& context) const{
    if(!m_root) return std::string{};

    BTResult result = m_root->tick(context);

    return (!result.success) ? std::string{} : std::move(result.command);
}

