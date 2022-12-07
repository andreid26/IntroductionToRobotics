# Introduction to Robotics (2022 - 2023)
Introduction to Robotics laboratory homeworks, taken in the 3rd year at the Faculty of Mathematics and Computer Science, University of Bucharest. Each homework includes requirements, implementation details, code and image files.

## Homework 1
Use a separate potentiometer in controlling each of the color of the RGB led (Red, Green and Blue).  The control must be done with digital electronics.

<img src="https://user-images.githubusercontent.com/63780942/197807340-74619d9c-2af2-4e26-bccd-e27cbba30da3.png" style="width: 50%;"/>
Demo: https://youtu.be/PYFzZjz3eTs

## Homework 2
Building the traffic lights for a crosswalk using: 2 LEDs to represent the traffic lights for people (red and green) and 3 LEDs to represent the traffic lights for cars (red, yellow and green).

<img src="https://user-images.githubusercontent.com/63780942/199254066-5ec5c23a-67e5-4c3c-aae4-6f16922f32d7.png" style="width: 50%;"/>
Demo: https://youtu.be/DceG4eFD4A0

## Homework 3
With a 7-segment display, use the joystick to control the position ofthe segment and "draw" on the display. The movement between segments should be natural (meaning they should jump from the current position only to neighbors, but without passing through "walls").

<img src="https://user-images.githubusercontent.com/63780942/200663670-025281e6-423b-4935-be09-e2bf6b3d9fc0.png" style="width: 50%;"/>
Demo: https://youtu.be/iPd-bjtZ1IY

## Homework 4
Use the joystick to move through the 4 digit 7-segment displays digits, press the button to lock in on the current digit and use the other axis to increment or decrement the number. Keep the button pressed to reset all the digit values and the current position to the first digit in the first state.

<img src="https://user-images.githubusercontent.com/63780942/201972968-9a52cb03-85f0-45c5-829c-ac6b9f0a70cd.png" style="width: 50%;"/>
Demo: https://youtu.be/GnH7jFEcMfk

## Homework 5

Instructions

    1. When powering up the game, a greeting message is shown until the joystick button is pressed
    2. Contains the following categories:
            (a) Start game - when pressed, the game starts with the default difficulty (Low)
            (b) Highscore:
                    - initially, it prints the saved value from the EEPROM
                    - when a player makes a new highscore, the value is updated and saved to EEPROM
            (c) Settings:
                    - Name: when locked (i.e the joystick button was pressed) reads the value from Serial and store it
                    - Difficuly: there are 3 different values (LOW, MEDIUM, HIGH)
                    - Sound: ON or OFF - saved to EEPROM
                    - Matrix brightness - from 0 to 15 - saved to EEPROM
                    - LCD brightness - from 0 to 255 - saved to EEPROM
                    - Back - when pressed, returns to the main menu
            (d) Info: includes details about the github profile
            (e) How to play: short and informative description
    3. The navigation through the menu is done by moving the joystick UP and DOWN

In this game, the player uses the joystick to move the main LED. At the same time, random generated food is placed on the matrix. As the main LED goes through the food, the score is increased:

    - For LOW difficulty: +1 for every 3 pieces of food
    - For MEDIUM difficulty: +1 for every 2 pieces of food
    - For HIGH difficulty: +1 for every piece of food
    
Increasing the difficulty makes the main LED move faster.

<img src="https://user-images.githubusercontent.com/63780942/206270434-feaf471d-c56b-4979-8cb6-9ad87cee0516.png" style="width: 50%;"/>
Demo: https://youtu.be/2HMOlzzKrJI
