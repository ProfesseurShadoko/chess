#include <iostream>
#include <Eigen/Dense>
#include <SFML/Graphics.hpp>
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
    test.complete(1+1 == 2);

    Test test2("Checking multiplication");
    test2.complete(2*2 == 3);

    return 0;
}