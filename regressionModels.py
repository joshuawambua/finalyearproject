from sklearn.linear_model import LinearRegression
from sklearn.ensemble import RandomForestRegressor
from sklearn.metrics import mean_absolute_error, mean_squared_error, r2_score
import tensorflow as tf
from tensorflow.keras.models import Sequential
from tensorflow.keras.layers import LSTM, Dense

def train_regression_models(X_train, X_test, y_train, y_test):
    """
    Train and compare multiple regression models for pollution forecasting
    """
    models = {}
    results = {}
    
    # 1. Linear Regression (Baseline)
    lr = LinearRegression()
    lr.fit(X_train, y_train)
    models['Linear Regression'] = lr
    
    # 2. Random Forest Regressor
    rf = RandomForestRegressor(n_estimators=100, random_state=42)
    rf.fit(X_train, y_train)
    models['Random Forest'] = rf
    
    # Evaluate all models
    for name, model in models.items():
        y_pred = model.predict(X_test)
        results[name] = {
            'MAE': mean_absolute_error(y_test, y_pred),
            'RMSE': np.sqrt(mean_squared_error(y_test, y_pred)),
            'R²': r2_score(y_test, y_pred)
        }
    
    return models, results

# Prepare data for regression (predict next hour's PM2.5)
df['pm25_next_hour'] = df['pm25'].shift(-1)  # Target variable
df = df.dropna()

X = df[features]
y = df['pm25_next_hour']

# Split data
X_train, X_test, y_train, y_test = train_test_split(X, y, test_size=0.2, random_state=42)

# Train models
models, results = train_regression_models(X_train, X_test, y_train, y_test)

# Print results
print("=== Regression Model Performance ===")
for model_name, metrics in results.items():
    print(f"\n{model_name}:")
    print(f"  MAE: {metrics['MAE']:.2f}")
    print(f"  RMSE: {metrics['RMSE']:.2f}")
    print(f"  R²: {metrics['R²']:.2f}")
