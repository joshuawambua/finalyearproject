def create_lstm_model(sequence_length, n_features):
    """Create LSTM model for time series forecasting"""
    model = Sequential([
        LSTM(50, return_sequences=True, input_shape=(sequence_length, n_features)),
        LSTM(50, return_sequences=False),
        Dense(25),
        Dense(1)
    ])
    model.compile(optimizer='adam', loss='mse', metrics=['mae'])
    return model

def prepare_sequences(df, feature_columns, target_column, sequence_length=24):
    """Prepare data for LSTM sequences"""
    from sklearn.preprocessing import MinMaxScaler
    
    scaler = MinMaxScaler()
    scaled_data = scaler.fit_transform(df[feature_columns + [target_column]])
    
    X, y = [], []
    for i in range(sequence_length, len(scaled_data)):
        X.append(scaled_data[i-sequence_length:i, :-1])  # Features
        y.append(scaled_data[i, -1])  # Target
    
    return np.array(X), np.array(y), scaler

# Prepare LSTM data
feature_columns = ['temperature', 'humidity', 'pm25', 'pm10', 'mq135']
target_column = 'pm25_next_hour'

X_lstm, y_lstm, scaler = prepare_sequences(df, feature_columns, target_column)

# Split LSTM data
X_train_lstm, X_test_lstm, y_train_lstm, y_test_lstm = train_test_split(
    X_lstm, y_lstm, test_size=0.2, random_state=42
)

# Train LSTM model
lstm_model = create_lstm_model(sequence_length=24, n_features=len(feature_columns))
history = lstm_model.fit(X_train_lstm, y_train_lstm, 
                        epochs=50, batch_size=32, 
                        validation_split=0.2, verbose=1)

# Evaluate LSTM
lstm_pred = lstm_model.predict(X_test_lstm)
lstm_mae = mean_absolute_error(y_test_lstm, lstm_pred)
lstm_rmse = np.sqrt(mean_squared_error(y_test_lstm, lstm_pred))

print(f"\nLSTM Performance:")
print(f"MAE: {lstm_mae:.4f}, RMSE: {lstm_rmse:.4f}")
