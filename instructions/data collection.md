**Collection of Data**
To gather the training data for the machine learning model, I created a variety of vacuum-cleaner scenarios using gentle and playful techniques. For the “normal cleaning” label, I let the vacuum run in its everyday mode and logged vibration readings from the MPU6050 sensor. (Figure 1)To mimic a “clogged” situation without harming the unit, I placed an electric toothbrush against the Mpu6050 and measured the emitted frequency. (Figure 2) When the vacuum was plugged in, I captured the vibration signatures during the battery charge cycle for the “charging” label. (Figure 3) I also logged observations when the vacuum was powered off, ensuring that I recorded its calm, stationary state.
I aimed for a consistent and representative dataset, so I acquired a total of 10 samples per class. Each recording ran for 30 seconds, proportioned collection of data to train the machine learning model effectively.

<img width="603" height="452" alt="image" src="https://github.com/user-attachments/assets/db4fc3b8-387f-431e-8858-f0cd5f88270f" />
<img width="418" height="453" alt="image" src="https://github.com/user-attachments/assets/c161ab45-7d54-4092-a0e5-038f8469f8d9" />
<img width="331" height="442" alt="image" src="https://github.com/user-attachments/assets/fa3f92dd-de64-4974-8469-ed65369ce431" />
