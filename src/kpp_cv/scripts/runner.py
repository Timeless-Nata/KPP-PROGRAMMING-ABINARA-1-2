import time, cv2, rospy
from sensor_msgs.msg import Image
from cv_bridge import CvBridge

def run_node(name, process):
    rospy.init_node(name)
    topic = rospy.get_param("~topic", "/camera/image_raw")
    bridge = CvBridge()
    st = {"t": time.time(), "fps": 0.0}

    def cb(msg):
        frame = bridge.imgmsg_to_cv2(msg, "bgr8")
        out = process(frame)
        now = time.time()
        st["fps"] = 0.9 * st["fps"] + 0.1 / max(now - st["t"], 1e-6)
        st["t"] = now
        cv2.putText(out, f"FPS: {st['fps']:.1f}", (10, 30),
                    cv2.FONT_HERSHEY_SIMPLEX, 0.8, (0, 255, 0), 2)
        cv2.imshow(name, out)
        if cv2.waitKey(1) == 27:  # ESC
            rospy.signal_shutdown("ESC")

    rospy.Subscriber(topic, Image, cb, queue_size=1, buff_size=2**24)
    rospy.spin()
    cv2.destroyAllWindows()
