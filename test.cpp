#include <iostream>
#include <vector>
#include <string>
#include <algorithm>

class Person {
private:
    std::string name;
    int age;

public:
    Person(const std::string& n, int a) : name(n), age(a) {}

    void birthday() {
        age++;
        std::cout << name << " is now " << age << " years old!" << std::endl;
    }

    std::string getName() const { return name; }
    int getAge() const { return age; }
};

// Function to demonstrate templates
template<typename T>
T findMax(const std::vector<T>& items) {
    if (items.empty()) {
        throw std::runtime_error("Vector is empty!");
    }
    return *std::max_element(items.begin(), items.end());
}

int main() {
    // Create some people
    std::vector<Person> people;
    people.push_back(Person("Alice", 25));
    people.push_back(Person("Bob", 30));
    people.push_back(Person("Charlie", 35));

    // Test the Person class
    std::cout << "Testing Person class:" << std::endl;
    for (auto& person : people) {
        std::cout << person.getName() << " is " << person.getAge() << " years old" << std::endl;
        person.birthday();
    }

    // Test template function with different types
    std::cout << "\nTesting template function:" << std::endl;
    
    std::vector<int> numbers = {1, 5, 3, 8, 2, 9, 4};
    std::cout << "Max number: " << findMax(numbers) << std::endl;

    std::vector<std::string> words = {"apple", "banana", "cherry", "date"};
    std::cout << "Last word alphabetically: " << findMax(words) << std::endl;

    // Test some C++11 features
    std::cout << "\nTesting modern C++ features:" << std::endl;
    
    // Lambda function
    auto printSquare = [](int x) { 
        std::cout << x << " squared is " << x * x << std::endl; 
    };

    std::cout << "Using lambda function:" << std::endl;
    for (int i = 1; i <= 5; i++) {
        printSquare(i);
    }

    for(int i = 6; i <= 10; i++){
      printSquare(i);
    }

    for(int i = 11; i <= 15; i++){
      std::cout << i << " squared is " << i * i << std::endl;
    }

    std::cout << "Finished testing modern C++ features." << std::endl;

    return 0;
}
