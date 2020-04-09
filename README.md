# ESP8266-NodeMCU - display realtime COVID-19 Data
Display Covid-19 data in real time using ESP8266-NodeMCU and 128x64 OLED

Thanks to Saswat Samal for inspiring me. His work is on Hackster:

https://www.hackster.io/SaswatSamal03/covid19-realtime-tracker-india-6e54e4?utm_campaign=new_projects&utm_content=7&utm_medium=email&utm_source=hackster&utm_term=project_image

and

https://www.youtube.com/watch?v=3Vf-nX3JOTQ


## Getting Started
You'll need an ESP8266-NodeMCU, and OLED Screen (I used the 128x64) 4 female/female jumper cables, USB cable for power and data to ESP8266.

The attached .ino file will get you up and running. The provided jpeg file: data_display_oled.jpg shows how the data will be displayed on the OLED.

Good Luck...




Version 2: April 08, 2020

Made extensive changes to the code and functionaliry:
	Added a button to trigger the updating of data. Earlier version was too invasive on the data source website. With the help of a button and restricting data updates to once every hour (between button clicks) - data is now fetched ONLY on demand.
While the OLED screen is waiting for a "button click"/request for data - a screen saver kicks in with the text "Press button to update". This text scrolls around randomly across the screen.
An interrup service routine traps the click of the button and processes the request - either displays the cached data (if less than 1 hr has elapsed since last refresh of data) or gets updated data if more than 1 hr has elapsed since last update.

2 pictures show the new screen and breadboard connections.
