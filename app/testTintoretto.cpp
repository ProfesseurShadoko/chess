#include <iostream>
#include <Eigen/Dense>
// #include <SFML/Graphics.hpp>
#include <tintoretto.hpp>

int main() {
    Message("Hello, World!", "#");

    // let's try to mute the message
    Message::mute();
    Message("This should not be printed!", "!");
    Message::unmute();
    Message("This should be printed!");

    Message::tab();
    Message("This should be tabbed!");
    Message::tab();
    Message("This should be double tabbed!");
    Message::untab();
    Message::unmute();

    Task task("Computing Stuff");
    Task::sleep(1000);
    task.complete();

    // let's try the progress bar also

    int N = 100;
    task = Task("Computing many things");

    ProgressBar bar(N);
    for (int i=0; i<N; i++) {
        Task::sleep(20);
        bar.update();

        if (i == 33) {
            bar.whisper("We reach 33!");
        }
    }

    task.complete();

    // let's try the test class
    Test test("Checking addition");
    test.check("1+1=2", 1+1==2);
    test.check("1+2=3", 1+2==3);
    test.complete();

    Test test2("Checking nothing");
    test2.complete();

    Test test3("Testing something that doesn't work");
    test3.check("0o1 = 1", 0+1==1);
    test3.check("1o1 = 11", 1+1==1);
    test3.complete();

    Message::par();
    // let's try out highlighting
    std::cout << cstr("This text is highlighted in green").h_green() << std::endl;
    std::cout << cstr("This text is red highlighted in blue").h_blue().red() << std::endl;
    Message("This might not work in some terminals, when text is highlighted some temrinals overwrite all text colors. VS code for instance.");


    // let's print all colors one by one
    Message::print("Printing all colors:");
    Message::tab();
    Message::print(cstr("Green").h_green());
    Message::print(cstr("Red").h_red());
    Message::print(cstr("Yellow").h_yellow().yellow());
    Message::print(cstr("Yellow").h_yellow().yellow());

    Message::print(cstr("Blue").h_blue());
    Message::print(cstr("Purple").h_purple());
    Message::print(cstr("Cyan").h_cyan());

    return 0;
}