from sklearn.ensemble import IsolationForest
from sklearn.metrics import precision_score, recall_score, f1_score

def detect_air_quality_anomalies(df, contamination=0.1):
    """
    Detect anomalous air quality readings using Isolation Forest
    """
    # Features for anomaly detection
    anomaly_features = ['pm25', 'pm10', 'mq135', 'mq7', 'mq2']
    X_anomaly = df[anomaly_features]
    
    # Handle missing values
    X_anomaly = X_anomaly.fillna(method='ffill')
    
    # Scale features
    scaler = StandardScaler()
    X_scaled = scaler.fit_transform(X_anomaly)
    
    # Train Isolation Forest
    iso_forest = IsolationForest(contamination=contamination, random_state=42)
    anomalies = iso_forest.fit_predict(X_scaled)
    
    # Convert predictions (1 = normal, -1 = anomaly)
    df['is_anomaly'] = anomalies
    df['is_anomaly'] = df['is_anomaly'].map({1: 0, -1: 1})  # 0=normal, 1=anomaly
    
    print(f"Detected {df['is_anomaly'].sum()} anomalies in the dataset")
    
    return df, iso_forest, scaler

# Detect anomalies
df_with_anomalies, iso_model, anomaly_scaler = detect_air_quality_anomalies(df)

# Visualize anomalies
plt.figure(figsize=(12, 6))
plt.plot(df_with_anomalies['timestamp'], df_with_anomalies['pm25'], 
         label='PM2.5', alpha=0.7)
plt.scatter(df_with_anomalies[df_with_anomalies['is_anomaly'] == 1]['timestamp'],
           df_with_anomalies[df_with_anomalies['is_anomaly'] == 1]['pm25'],
           color='red', label='Anomalies', s=50)
plt.title('PM2.5 Readings with Anomalies Detected')
plt.xlabel('Time')
plt.ylabel('PM2.5 (μg/m³)')
plt.legend()
plt.xticks(rotation=45)
plt.tight_layout()
plt.show()
