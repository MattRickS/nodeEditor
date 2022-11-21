#include <iostream>

#include "../src/nodeeditor/nodegraph/iterators.h"
#include "../src/nodeeditor/nodegraph/node.h"

int main()
{
    /*
              8
             /
    1   2   3
     \ /   /
      4   5
     / \ /
    6   7
    */
    Node n1 = Node(1, nullptr);
    Node n2 = Node(2, nullptr);
    Node n3 = Node(3, nullptr);
    Node n4 = Node(4, nullptr);
    Node n5 = Node(5, nullptr);
    Node n6 = Node(6, nullptr);
    Node n7 = Node(7, nullptr);
    Node n8 = Node(8, nullptr);

    n1.addOutput();
    n2.addOutput();
    n3.addOutput();
    n4.addOutput();
    n5.addOutput();
    n8.addOutput();
    n3.addInput();
    n4.addInput();
    n4.addInput();
    n5.addInput();
    n6.addInput();
    n7.addInput();
    n7.addInput();

    // TODO: Assert all true
    std::cout << "Connected: " << n3.input(0)->connect(n8.output(0)) << std::endl;
    std::cout << "Connected: " << n4.input(0)->connect(n1.output(0)) << std::endl;
    std::cout << "Connected: " << n4.input(1)->connect(n2.output(0)) << std::endl;
    std::cout << "Connected: " << n5.input(0)->connect(n3.output(0)) << std::endl;
    std::cout << "Connected: " << n6.input(0)->connect(n4.output(0)) << std::endl;
    std::cout << "Connected: " << n7.input(0)->connect(n4.output(0)) << std::endl;
    std::cout << "Connected: " << n7.input(1)->connect(n5.output(0)) << std::endl;

    // TODO: Assert n1 = 1
    // TODO: Assert n5 = 5,3,8
    // TODO: Assert n6 = 6,4,1,2
    // TODO: Assert n7 = 7,4,1,2,5,3,8
    DepthIterator end;
    for (DepthIterator di{&n7}; di != end; ++di)
    {
        std::cout << "Node: " << di->id() << ", depth: " << di.depth() << std::endl;
    }

    // TODO: test downstream
    return 0;
}
