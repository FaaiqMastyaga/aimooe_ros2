# Tool.msg

string tool_name
int16 tool_type
bool is_valid

# Accuracy Metrics
float32 mean_abs_error
float32 rms_error

# Pose Data
geometry_msgs/Vector3 rotation_vector
geometry_msgs/Quaternion quaternion
float32[9] rotation_matrix  # Flattened 3x3 matrix
geometry_msgs/Vector3 translation_vector
geometry_msgs/Point32 origin_coordinates

# Raw Marker Points (X, Y, Z for each marker flattened )
geometry_msgs/Point32[] marker_points