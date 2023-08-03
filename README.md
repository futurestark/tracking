# tracking
Testing tracking algorithms

OpenCV 4 comes with a tracking API that contains implementations of many single object tracking algorithms. There are 8 different trackers available in OpenCV 4.2 — BOOSTING, MIL, KCF, TLD, MEDIANFLOW, GOTURN, MOSSE, and CSRT.

Project based on OpenCV 4.2

to compile from command line:     g++ Tracking.cpp -o Tracking `pkg-config --cflags --libs opencv4`

How to run:
binary name, -i input_video_file_path -o ouptut_video_file_path -l log_output_file_path t- tracker_type -r percentage   (if -r is not set, default 5% will apply)
example:
./Tracking -i /home/jarvis/anyway_projects/ISR_NAU/Video_test_CV_10sec.mp4 -o /home/jarvis/anyway_projects/ISR_NAU/ -l /home/jarvis/anyway_projects/ISR_NAU -t KCF -r 5

------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
Object Tracking Algorithms:
------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

The motion model predicts the approximate location of the object. The appearance model fine tunes this estimate to provide a more accurate estimate based on appearance.

If the object was very simple and did not change it’s appearance much, we could use a simple template as an appearance model and look for that template. However, real life is not that simple. The appearance of an object can change dramatically. To tackle this problem, in many modern trackers, this appearance model is a classifier that is trained in an online manner. Don’t panic! Let me explain in simpler terms.

The job of the classifier is to classify a rectangular region of an image as either an object or background. The classifier takes in an image patch as input and returns a score between 0 and 1 to indicate the probability that the image patch contains the object. The score is 0 when it is absolutely sure the image patch is the background and 1 when it is absolutely sure the patch is the object.

A classifier is trained by feeding it positive ( object ) and negative ( background ) examples. If you want to build a classifier for detecting cats, you train it with thousands of images containing cats and thousands of images that do not contain cats. This way the classifier learns to differentiate what is a cat and what is not. You can learn more about image classification here. While building an online classifier, we do not have the luxury of having thousands of examples of the positive and negative classes.

BOOSTING Tracker
This tracker is based on an online version of AdaBoost — the algorithm that the HAAR cascade based face detector uses internally. This classifier needs to be trained at runtime with positive and negative examples of the object. The initial bounding box supplied by the user ( or by another object detection algorithm ) is taken as the positive example for the object, and many image patches outside the bounding box are treated as the background. Given a new frame, the classifier is run on every pixel in the neighborhood of the previous location and the score of the classifier is recorded. The new location of the object is the one where the score is maximum. So now we have one more positive example for the classifier. As more frames come in, the classifier is updated with this additional data.

Pros : None. This algorithm is a decade old and works ok, but I could not find a good reason to use it especially when other advanced trackers (MIL, KCF) based on similar principles are available.

Cons : Tracking performance is mediocre. It does not reliably know when tracking has failed.

MIL Tracker
This tracker is similar in idea to the BOOSTING tracker described above. The big difference is that instead of considering only the current location of the object as a positive example, it looks in a small neighborhood around the current location to generate several potential positive examples. You may be thinking that it is a bad idea because in most of these “positive” examples the object is not centered.

This is where Multiple Instance Learning ( MIL ) comes to rescue. In MIL, you do not specify positive and negative examples, but positive and negative “bags”. The collection of images in the positive bag are not all positive examples. Instead, only one image in the positive bag needs to be a positive example! In our example, a positive bag contains the patch centered on the current location of the object and also patches in a small neighborhood around it. Even if the current location of the tracked object is not accurate, when samples from the neighborhood of the current location are put in the positive bag, there is a good chance that this bag contains at least one image in which the object is nicely centered. MIL project page has more information for people who like to dig deeper into the inner workings of the MIL tracker.

Pros : The performance is pretty good. It does not drift as much as the BOOSTING tracker and it does a reasonable job under partial occlusion. If you are using OpenCV 3.0, this might be the best tracker available to you. But if you are using a higher version, consider KCF.

Cons : Tracking failure is not reported reliably. Does not recover from full occlusion.

KCF Tracker
KFC stands for Kernelized Correlation Filters. This tracker builds on the ideas presented in the previous two trackers. This tracker utilizes that fact that the multiple positive samples used in the MIL tracker have large overlapping regions. This overlapping data leads to some nice mathematical properties that is exploited by this tracker to make tracking faster and more accurate at the same time.

Pros: Accuracy and speed are both better than MIL and it reports tracking failure better than BOOSTING and MIL. If you are using OpenCV 3.1 and above, I recommend using this for most applications.

Cons : Does not recover from full occlusion. Not implemented in OpenCV 3.0.

TLD Tracker
TLD stands for Tracking, learning and detection. As the name suggests, this tracker decomposes the long term tracking task into three components — (short term) tracking, learning, and detection. From the author’s paper, “The tracker follows the object from frame to frame. The detector localizes all appearances that have been observed so far and corrects the tracker if necessary. The learning estimates detector’s errors and updates it to avoid these errors in the future.” This output of this tracker tends to jump around a bit. For example, if you are tracking a pedestrian and there are other pedestrians in the scene, this tracker can sometimes temporarily track a different pedestrian than the one you intended to track. On the positive side, this track appears to track an object over a larger scale, motion, and occlusion. If you have a video sequence where the object is hidden behind another object, this tracker may be a good choice.

Pros : Works the best under occlusion over multiple frames. Also, tracks best over scale changes.

Cons : Lots of false positives making it almost unusable.

MEDIANFLOW Tracker
Internally, this tracker tracks the object in both forward and backward directions in time and measures the discrepancies between these two trajectories. Minimizing this ForwardBackward error enables them to reliably detect tracking failures and select reliable trajectories in video sequences.

Pros : Excellent tracking failure reporting. Works very well when the motion is predictable and there is no occlusion.
Cons : Fails under large motion.

GOTURN tracker
Out of all the tracking algorithms in the tracker class, this is the only one based on Convolutional Neural Network (CNN). From OpenCV documentation, we know it is “robust to viewpoint changes, lighting changes, and deformations”. But it does not handle occlusion very well.

Notice : GOTURN being a CNN based tracker, uses a caffe model for tracking. The Caffe model and the proto text file must be present in the directory in which the code is present. These files can also be downloaded from the opencv_extra repository, concatenated and extracted before use.

MOSSE tracker
Minimum Output Sum of Squared Error (MOSSE) uses adaptive correlation for object tracking which produces stable correlation filters when initialized using a single frame. MOSSE tracker is robust to variations in lighting, scale, pose, and non-rigid deformations. It also detects occlusion based upon the peak-to-sidelobe ratio, which enables the tracker to pause and resume where it left off when the object reappears. MOSSE tracker also operates at a higher fps (450 fps and even more). To add to the positives, it is also very easy to implement, is as accurate as other complex trackers and much faster. But, on a performance scale, it lags behind the deep learning based trackers.
CSRT tracker

In the Discriminative Correlation Filter with Channel and Spatial Reliability (DCF-CSR), we use the spatial reliability map for adjusting the filter support to the part of the selected region from the frame for tracking. This ensures enlarging and localization of the selected region and improved tracking of the non-rectangular regions or objects. It uses only 2 standard features (HoGs and Colornames). It also operates at a comparatively lower fps (25 fps) but gives higher accuracy for object tracking.
