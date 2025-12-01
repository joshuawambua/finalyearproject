def comprehensive_evaluation(models, X_test, y_test, problem_type='regression'):
    """
    Comprehensive evaluation of all models
    """
    evaluation_results = {}
    
    for model_name, model in models.items():
        y_pred = model.predict(X_test)
        
        if problem_type == 'regression':
            evaluation_results[model_name] = {
                'MAE': mean_absolute_error(y_test, y_pred),
                'RMSE': np.sqrt(mean_squared_error(y_test, y_pred)),
                'R²': r2_score(y_test, y_pred)
            }
        elif problem_type == 'classification':
            evaluation_results[model_name] = {
                'Accuracy': accuracy_score(y_test, y_pred),
                'Precision': precision_score(y_test, y_pred, average='weighted'),
                'Recall': recall_score(y_test, y_pred, average='weighted'),
                'F1-Score': f1_score(y_test, y_pred, average='weighted')
            }
    
    # Create comparison DataFrame
    results_df = pd.DataFrame(evaluation_results).T
    return results_df

# Evaluate regression models
regression_results = comprehensive_evaluation(models, X_test, y_test, 'regression')
print("\n=== Regression Models Comparison ===")
print(regression_results)

# Evaluate classification models (if you have true labels for anomalies)
# classification_results = comprehensive_evaluation(classification_models, X_test, y_test, 'classification')
