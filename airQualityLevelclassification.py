from sklearn.svm import SVC
from sklearn.ensemble import RandomForestClassifier
from sklearn.metrics import accuracy_score, classification_report, confusion_matrix
import seaborn as sns

def classify_air_quality_levels(df):
    """
    Classify air quality into categories based on WHO standards
    """
    # Define AQI categories based on PM2.5
    conditions = [
        (df['pm25'] <= 12),
        (df['pm25'] <= 35.4),
        (df['pm25'] <= 55.4),
        (df['pm25'] > 55.4)
    ]
    choices = ['Good', 'Moderate', 'Poor', 'Hazardous']
    df['aqi_category'] = np.select(conditions, choices)
    
    # Prepare features and target
    X = df[features]
    y = df['aqi_category']
    
    # Encode target labels
    le = LabelEncoder()
    y_encoded = le.fit_transform(y)
    
    # Split data
    X_train, X_test, y_train, y_test = train_test_split(X, y_encoded, test_size=0.2, random_state=42)
    
    # Scale features
    scaler = StandardScaler()
    X_train_scaled = scaler.fit_transform(X_train)
    X_test_scaled = scaler.transform(X_test)
    
    # Train classifiers
    classifiers = {
        'SVM': SVC(kernel='rbf', random_state=42),
        'Random Forest': RandomForestClassifier(n_estimators=100, random_state=42)
    }
    
    results = {}
    for name, clf in classifiers.items():
        clf.fit(X_train_scaled, y_train)
        y_pred = clf.predict(X_test_scaled)
        
        results[name] = {
            'model': clf,
            'accuracy': accuracy_score(y_test, y_pred),
            'predictions': y_pred,
            'true_labels': y_test
        }
        
        print(f"\n{name} Classification Report:")
        print(classification_report(y_test, y_pred, target_names=le.classes_))
    
    return results, le

# Run classification
classification_results, label_encoder = classify_air_quality_levels(df)

# Plot confusion matrix for best model
best_model_name = max(classification_results.keys(), 
                     key=lambda x: classification_results[x]['accuracy'])

y_true = classification_results[best_model_name]['true_labels']
y_pred = classification_results[best_model_name]['predictions']

plt.figure(figsize=(8, 6))
cm = confusion_matrix(y_true, y_pred)
sns.heatmap(cm, annot=True, fmt='d', cmap='Blues',
            xticklabels=label_encoder.classes_,
            yticklabels=label_encoder.classes_)
plt.title(f'Confusion Matrix - {best_model_name}')
plt.ylabel('Actual')
plt.xlabel('Predicted')
plt.show()
