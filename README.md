[![GitHub license](https://img.shields.io/badge/license-MIT-blue.svg)](https://raw.githubusercontent.com/bsy6766/BehaviorTree/master/LICENSE) ![](https://img.shields.io/badge/language-C%2B%2B11-brightgreen.svg)

# BehaviorTree
Simple implementation of BehaviorTree with C++.

## Node State
- SUCCESS: Node has successfully executed and succeeded.
- FAILURE: Node has successfully executed and failed. 
- RUNNING: Node has successfully executed and still running.
- ERROR: Node either had error on execute, while execution, no children, wrong condition, wrong variable or all possible invalid operations. This is different from FAILURE. By default, ERROR is considered as FAILURE.
Note: If you do not want to consider ERROR as FAILURE, set BehaviorTree::IGNORE_ERROR to false. Execution will stop immediately if this is set to false.

## Composite Nodes
Composite node can have one or more children. <br>
Each derived composite nodes (see below) will have their own rules on how and when excatly children get executed.<br>
Childrens are stored in std::vector and follows the FIFO rule, which means first child that got added will be the first child in order.

### Selector
Selector executes children sequently until one of them returns SUCCESS or RUNNING.

### Random Selector
Random Selector is same as Selector but shuffles children order on every excution only if random selector is not on running state.

### Sequence
Sequence executes children sequently if only child node successes.

### Random Sequence
Random Sequence is same as Sequence but shuffles children order on every excution only if random sequence is not on running state.

## Decorator Nodes
Decorator nodes decorates the behavior of execution or the results.

### Inverter
Inverter invertes node's result only if the result is either SUCCESS or FAILURE.

### Succeeder
Succeeder always return SUCCESS state no matter how execution resulted.

### Failer
Failer always return FAILURE state no matter how execution resulted.

### Repeater
Repeater repeats execution for certain amount of time only if result is SUCCESS or FAILURE.<br>

### RepeatUntilFailure
RepeatUntilFailure repeats until the execution results as FAILURE and return SUCCESS.<br>
The repetition can be long and fall in infinite loop. Recommanded to use thread.

### RepeatUntilSuccess
RepeatUntilFailure repeats until the execution results as SUCCESS and return SUCCESS.<br>
The repetition can be long and fall in infinite loop. Recommanded to use thread.

### Limiter
Limits the number or execution of the child node. Limiter will not execute child once the number of execution exceeds limit and return FAILURE as result.

### DelayTime
DelayTime delays time before child node gets executed. If child gets executed, it will be delayed again. If delayOnce is set to true, it will only delay once for a lifetime and return the same result.

### TimeLimit
Limits the time until the child node finishes the execution. If it fails to finish before time expires, it will return FAILURE.
