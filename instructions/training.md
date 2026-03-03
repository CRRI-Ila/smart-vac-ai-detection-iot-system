For splitting the collected dataset, I opted for an 80-20 split, where 80% of the data was used for training and 20% for testing. This is a common and effective approach in supervised learning, as it provides the model with enough data to learn patterns while reserving a separate portion to evaluate its performance on unseen data. This helps prevent overfitting . (Figure 1)
<img width="467" height="351" alt="image" src="https://github.com/user-attachments/assets/fef33018-0c03-4a01-8b8c-4804c0ed6a01" />

(Figure 1)
Creating a impulse
In Edge Impulse, I began the impulse design by choosing the X, Y, and Z axes from the MPU6050 sensor for input data. I configured the window length and stride to 1000 ms, allowing the model to check the vibration pattern once every second. With a sampling rate of 200 Hz, the system recorded 200 samples each second, giving a fine-grained look at the vibrations. I turned on zero-padding to keep each data segment the same length, even if some input was a bit shorter. This arrangement produced clean, uniform time-series data that works well for both frequency and classification tasks.
<img width="823" height="407" alt="image" src="https://github.com/user-attachments/assets/96b76c07-311d-4ecc-b981-33d0bb461423" />

Generating features
To create the features, I applied the Spectral Analysis module using the FFT engine to shift the raw time-series recordings into the frequency domain. Activating the Autotune parameter’s function allowed the pipeline to refine the filter design and FFT configuration automatically, optimizing them for the characteristics of my data. The autotune settled on a low-pass filter cutting off at 86.71 Hz and selected an FFT length of 32, capturing only the most informative frequency bands. I chose to log the spectrum to accentuate subtle variations across the different vacuum states, yielding a refined set of frequency features. This configuration ultimately supplied the classification model with clean, distinguishing features for training.
<img width="436" height="477" alt="image" src="https://github.com/user-attachments/assets/3087b7e2-62a8-4fac-aa8b-02d083fc8ecd" />
<img width="470" height="352" alt="image" src="https://github.com/user-attachments/assets/cf7f7863-dd2e-4e7c-b4fc-f22660917eda" />

<img width="314" height="298" alt="image" src="https://github.com/user-attachments/assets/6cd30385-1d0c-4e8f-be11-b1baf3d514bd" />

Classifying
For classification, I used a simple neural network architecture with two dense layers (20 and 10 neurons) to keep the model lightweight and efficient for edge deployment. I trained the model over 65 cycles with a low learning rate of 0.0005 to ensure stable convergence. To prevent overfitting, I relied on a balanced dataset (as verified using the Data Explorer) and evaluated the model using the confusion matrix. The matrix showed 100% accuracy on all four classes — Charge, Cleaning, Off, and Clogged — indicating strong generalization on the validation set. The F1 score and all metrics (ROC AUC, precision, recall) were 1.00, confirming that the model is not just memorizing patterns but learning meaningful features from the spectral data.

<img width="837" height="423" alt="image" src="https://github.com/user-attachments/assets/4f412962-2c4f-4abd-bb55-c9f9c3451ba0" />
<img width="532" height="461" alt="image" src="https://github.com/user-attachments/assets/c65b5f90-40a2-4620-81ed-74c90b0c3408" />

Deployment
After this I deployed it and downloaded a zip folder then I included its library from Arduino.
On the device, incoming motion data from the MPU6050 sensor was stored in buffers, matching the same sampling rate and window size used during training. Once a full window of data was collected, it was passed to the Edge Impulse inference engine, which ran the model and returned a classification label (e.g., Cleaning, Off, Charge, or Clogged).(fig 1)

<img width="1033" height="779" alt="image" src="https://github.com/user-attachments/assets/7b7b4710-c051-4f48-b971-dbbef21f650f" />

This label was then formatted into a JSON packet and sent wirelessly to the cloud via MQTT, where it was received and processed in Node-RED. Node-RED acted as a central controller — checking the state, triggering actions like WhatsApp notifications, and logging the results to Grafana for long-term monitoring. This deployment flow allowed for real-time, intelligent decision-making using a lightweight, embedded AI system.(fig2)

<img width="517" height="298" alt="image" src="https://github.com/user-attachments/assets/9889be83-d90c-4673-b278-4a73ab53c454" />

Further more some parts of the codes were written in non blocking manner to ensure the CPU can handle everything at once. For example the the mqtt publish, Inference Classification Loop, check if my button is pressed like the debouncing part
<img width="705" height="73" alt="image" src="https://github.com/user-attachments/assets/157c88eb-8a44-40ef-8d75-be15b5d9a662" />
<img width="987" height="61" alt="image" src="https://github.com/user-attachments/assets/bcec9c53-a768-4662-b559-9a194282f119" />
<img width="1031" height="167" alt="image" src="https://github.com/user-attachments/assets/0f5c679e-7248-4b66-97ad-5c5a67b79f64" />



